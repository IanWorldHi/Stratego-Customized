module;

#include <curses.h>

module view;

import <iostream>;
import <sstream>;
import <string>;
import <cctype>;
import game;
import types;
import board;
import link;
import player;
import ability;
import xwindow;

using namespace std;

// helper to choose which player's perspective to use for visibility
static PlayerId viewerFor(const Game &game)
{
    return game.currentPlayer();
}

// formatLinkDetails prints "x: V1" or "x: ?" for a single link slot
// formatLinkDetails handles visibility rules so the view only shows info known to the viewer
static void formatLinkDetails(const Game &game, PlayerId viewer, PlayerId owner, int slot, ostream &out)
{
    const PlayerState &ps = game.getPlayer(owner);
    int linkIdx = ps.getLinkIndex(slot);

    char label = (owner == PlayerId::P1)
                     ? static_cast<char>('a' + slot)
                     : static_cast<char>('A' + slot);

    out << label << ": ";

    if (linkIdx < 0)
    {
        // downloaded or missing link
        out << ".";
        return;
    }

    const Link &link = game.getLink(linkIdx);

    bool reveal = false;

    if (owner == viewer)
    {
        // a player always sees their own links
        reveal = true;
    }
    else
    {
        // otherwise only show details if this link is known to the viewer
        reveal = link.isKnownBy(viewer);
    }

    if (!reveal)
    {
        out << "?";
        return;
    }

    char typeChar = (link.getKind() == LinkKind::Virus) ? 'V' : 'D';
    int strength = link.getStrength();

    out << typeChar << strength;
}

// printPlayerSection writes one player's summary block (downloads, abilities, links)
static void printPlayerSection(const Game &game, PlayerId who, ostream &out)
{
    PlayerId viewer = viewerFor(game);
    const PlayerState &ps = game.getPlayer(who);
    const PlayerAbilities &pa = game.getAbilities(who);

    int playerNum = (who == PlayerId::P1) ? 1 : 2;

    out << "Player " << playerNum << ": " << endl;
    out << "Downloaded: " << ps.getDownloadedData()
        << "D, " << ps.getDownloadedVirus() << "V" << endl;
    out << "Abilities: " << pa.remaining() << endl;

    // two rows of four links each
    for (int row = 0; row < 2; ++row)
    {
        int startSlot = row * 4;
        int endSlot = startSlot + 4;

        for (int slot = startSlot; slot < endSlot; ++slot)
        {
            formatLinkDetails(game, viewer, who, slot, out);
            if (slot + 1 < endSlot)
            {
                out << " ";
            }
        }
        out << endl;
    }
}

// printBoardRows writes the 8x8 board grid using the display rules from the spec
static void printBoardRows(const Game &game, ostream &out)
{
    const Board &board = game.board();

    for (int r = 0; r < 8; ++r)
    {
        for (int c = 0; c < 8; ++c)
        {
            Position p{r, c};
            const Cell &cell = board.at(p);

            char ch = '.';

            if (cell.getKind() == CellKind::Link)
            {
                int idx = cell.getLinkIndex();
                const Link &lnk = game.getLink(idx);
                ch = lnk.getLabel();
            }
            else if (cell.firewallPresent())
            {
                PlayerId owner = cell.getFirewallOwner();
                // m for player 1 firewalls, w for player 2 firewalls
                ch = (owner == PlayerId::P1) ? 'm' : 'w';
            }
            else if (cell.isServerPortFor(PlayerId::P1) ||
                     cell.isServerPortFor(PlayerId::P2))
            {
                ch = 'S';
            }

            out << ch;
        }
        out << endl;
    }
}

// ==================== TextView ====================

// showBoard prints the full display with player 1 on top and player 2 on the bottom
void TextView::showBoard(const Game &game)
{
    cout << "=========================" << endl;

    printPlayerSection(game, PlayerId::P1, cout);

    cout << "========" << endl;
    printBoardRows(game, cout);
    cout << "========" << endl;

    printPlayerSection(game, PlayerId::P2, cout);

    cout << "=========================" << endl;
}

// showMessage prints a single line as-is
void TextView::showMessage(const string &msg)
{
    cout << msg << endl;
}

// showPrompt prints a prompt (with leading space) and flushes so the user sees it
void TextView::showPrompt(const string &prompt)
{
    cout << " " << prompt;
    cout.flush();
}

// readCommand reads one full command line from stdin
string TextView::readCommand()
{
    string line;
    if (!getline(cin, line))
    {
        // Treat EOF as a request to quit
        return "quit";
    }
    return line;
}

// ==================== CursesView ====================

CursesView::CursesView()
{
    // initialize ncurses once when the curses view is constructed
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    inputBuffer[0] = '\0';
}

CursesView::~CursesView()
{
    // restore terminal state
    endwin();
}

void CursesView::showBoard(const Game &game)
{
    // render into a string first so we can dump it with a single printw
    ostringstream out;

    out << "=========================" << '\n';
    printPlayerSection(game, PlayerId::P1, out);
    out << "========" << '\n';
    printBoardRows(game, out);
    out << "========" << '\n';
    printPlayerSection(game, PlayerId::P2, out);
    out << "=========================" << '\n';

    string s = out.str();

    // only 1 board (avoid stacking) refresh, clear, refresh, then print the new board
    refresh();
    clear();
    refresh();
    move(0, 0); // reset print location to top left
    printw("%s", s.c_str());
    refresh();
}

void CursesView::showMessage(const string &msg)
{
    printw("%s\n", msg.c_str());
    refresh();
}

void CursesView::showPrompt(const string &prompt)
{
    printw(" %s", prompt.c_str());
    refresh();
}

string CursesView::readCommand()
{
    // read one line of input after prompt is printed
    echo();
    int rc = getnstr(inputBuffer, static_cast<int>(sizeof(inputBuffer) - 1));
    noecho();

    if (rc == ERR)
    {
        // treat error/EOF as quit
        return "quit";
    }

    return string{inputBuffer};
}

// ==================== XView (X11) ====================

XView::XView()
    : window{XWIN_WIDTH, XWIN_HEIGHT},
      cellSize{40},
      boardOriginX{60},
      boardOriginY{120},
      statusHeight{60},
      lastMessage{},
      lastPrompt{},
      abilitiesActive{false},
      abilityLinesDrawn{0},
      abilityHeader{},
      abilitiesOwner{PlayerId::None},
      lastBoardPlayer{PlayerId::None}
{
    for (int i = 0; i < 5; ++i)
    {
        abilityLines[i].clear();
    }
}

void XView::clearAll()
{
    // fill entire window with white
    window.fillRectangle(0, 0, XWIN_WIDTH, XWIN_HEIGHT, Xwindow::White);
}

void XView::drawGrid()
{
    // simple white squares; links and markers will be drawn on top
    for (int r = 0; r < 8; ++r)
    {
        for (int c = 0; c < 8; ++c)
        {
            int x = boardOriginX + c * cellSize;
            int y = boardOriginY + r * cellSize;
            window.fillRectangle(x, y, cellSize - 1, cellSize - 1, Xwindow::White);
        }
    }
}

int XView::colourForLink(const Link &link, PlayerId viewer) const
{
    bool known = (link.getOwner() == viewer) || link.isKnownBy(viewer);

    if (!known)
    {
        // unknown links are rendered in black
        return Xwindow::Black;
    }

    if (link.getKind() == LinkKind::Data)
    {
        return Xwindow::Green; // Link
    }

    return Xwindow::Red; // Virus
}

void XView::drawLinks(const Game &game)
{
    const Board &board = game.board();
    PlayerId viewer = viewerFor(game);

    for (int r = 0; r < 8; ++r)
    {
        for (int c = 0; c < 8; ++c)
        {
            Position p{r, c};
            const Cell &cell = board.at(p);

            int x = boardOriginX + c * cellSize;
            int y = boardOriginY + r * cellSize;

            // base background for the cell
            window.fillRectangle(x, y, cellSize - 1, cellSize - 1, Xwindow::White);

            if (cell.getKind() == CellKind::Link)
            {
                int idx = cell.getLinkIndex();
                const Link &lnk = game.getLink(idx);

                int colour = colourForLink(lnk, viewer);
                window.fillRectangle(x, y, cellSize - 1, cellSize - 1, colour);

                string labelStr(1, lnk.getLabel());
                // draw label roughly centred
                int textX = x + cellSize / 3;
                int textY = y + (2 * cellSize) / 3;
                window.drawString(textX, textY, labelStr);
            }
            else if (cell.firewallPresent())
            {
                // render firewall marker
                PlayerId owner = cell.getFirewallOwner();
                char fw = (owner == PlayerId::P1) ? 'm' : 'w';
                string fwStr(1, fw);
                int textX = x + cellSize / 3;
                int textY = y + (2 * cellSize) / 3;
                window.drawString(textX, textY, fwStr);
            }
            else if (cell.isServerPortFor(PlayerId::P1) ||
                     cell.isServerPortFor(PlayerId::P2))
            {
                string sStr("S");
                int textX = x + cellSize / 3;
                int textY = y + (2 * cellSize) / 3;
                window.drawString(textX, textY, sStr);
            }
            // empty cells stay as white squares
        }
    }
}

void XView::drawPlayerPanel(const Game &game, PlayerId who, bool top)
{
    PlayerId viewer = viewerFor(game);
    const PlayerState &ps = game.getPlayer(who);
    const PlayerAbilities &pa = game.getAbilities(who);

    int playerNum = (who == PlayerId::P1) ? 1 : 2;

    int baseY = top ? 30 : (boardOriginY + 8 * cellSize + 20);
    int x = 20;

    ostringstream line;

    line << "Player " << playerNum << ":";
    window.drawString(x, baseY, line.str());
    line.str("");
    line.clear();

    line << "Downloaded: " << ps.getDownloadedData()
         << "D, " << ps.getDownloadedVirus() << "V";
    window.drawString(x, baseY + 15, line.str());
    line.str("");
    line.clear();

    line << "Abilities: " << pa.remaining();
    window.drawString(x, baseY + 30, line.str());
    line.str("");
    line.clear();

    // two rows of four links each
    for (int row = 0; row < 2; ++row)
    {
        int startSlot = row * 4;
        int endSlot = startSlot + 4;

        for (int slot = startSlot; slot < endSlot; ++slot)
        {
            formatLinkDetails(game, viewer, who, slot, line);
            if (slot + 1 < endSlot)
            {
                line << " ";
            }
        }

        window.drawString(x, baseY + 45 + row * 15, line.str());
        line.str("");
        line.clear();
    }
}

// showBoard draws the full board and both player panels
void XView::showBoard(const Game &game)
{
    PlayerId current = game.currentPlayer();

    // if the turn has changed hands, wipe any cached abilities overlay
    if (lastBoardPlayer != PlayerId::None && lastBoardPlayer != current)
    {
        abilitiesActive = false;
        abilitiesOwner = PlayerId::None;
        abilityHeader.clear();
        abilityLinesDrawn = 0;
        for (int i = 0; i < 5; ++i)
        {
            abilityLines[i].clear();
        }
    }
    lastBoardPlayer = current;

    clearAll();
    drawGrid();
    drawLinks(game);
    drawPlayerPanel(game, PlayerId::P1, true);
    drawPlayerPanel(game, PlayerId::P2, false);

    int p2BaseY = boardOriginY + 8 * cellSize + 20;

    // place the message band below the P2 legend lines
    int msgY = p2BaseY + 80;
    int abilitiesStartY = msgY + 20;

    // redraw the message band from lastMessage
    window.fillRectangle(0, msgY - 15, XWIN_WIDTH, 20, Xwindow::White);
    if (!lastMessage.empty())
    {
        window.drawString(20, msgY, lastMessage);
    }

    // redraw abilities overlay only if it belongs to the current player
    window.fillRectangle(0, abilitiesStartY - 15, XWIN_WIDTH, 20 * 6, Xwindow::White);

    if (abilitiesActive && abilitiesOwner == current)
    {
        if (!abilityHeader.empty())
        {
            window.drawString(20, abilitiesStartY, abilityHeader);
        }

        for (int i = 0; i < abilityLinesDrawn && i < 5; ++i)
        {
            if (!abilityLines[i].empty())
            {
                int y = abilitiesStartY + 15 * (i + 1);
                window.drawString(20, y, abilityLines[i]);
            }
        }
    }
}

// showMessage draws or updates the message line below the board
void XView::showMessage(const string &msg)
{
    int p2BaseY = boardOriginY + 8 * cellSize + 20;
    int msgY = p2BaseY + 80;
    int abilitiesStartY = msgY + 20;

    // abilities header "Abilities:"
    if (msg.rfind("Abilities:", 0) == 0)
    {
        abilitiesActive = true;
        abilityHeader = msg;
        abilityLinesDrawn = 0;
        for (int i = 0; i < 5; ++i)
        {
            abilityLines[i].clear();
        }

        // figure out which player's abilities we are showing based on lastPrompt
        PlayerId owner = PlayerId::None;
        if (lastPrompt.size() >= 2 && lastPrompt[0] == 'P')
        {
            if (lastPrompt[1] == '1')
            {
                owner = PlayerId::P1;
            }
            else if (lastPrompt[1] == '2')
            {
                owner = PlayerId::P2;
            }
        }
        abilitiesOwner = owner;

        // draw the header into the abilities region
        window.fillRectangle(0, abilitiesStartY - 15, XWIN_WIDTH, 20 * 6, Xwindow::White);
        window.drawString(20, abilitiesStartY, abilityHeader);
        return;
    }

    // abilities lines
    if (abilitiesActive &&
        msg.size() >= 2 &&
        std::isdigit(static_cast<unsigned char>(msg[0])) &&
        msg[1] == ':')
    {
        int slot = msg[0] - '1';
        if (slot >= 0 && slot < 5)
        {
            abilityLines[slot] = msg;
            if (slot + 1 > abilityLinesDrawn)
            {
                abilityLinesDrawn = slot + 1;
            }

            int y = abilitiesStartY + 15 * (slot + 1);
            window.drawString(20, y, msg);
        }
        return;
    }

    // any other message is treated as a normal status line
    lastMessage = msg;

    // update the message band
    window.fillRectangle(0, msgY - 15, XWIN_WIDTH, 20, Xwindow::White);
    window.drawString(20, msgY, msg);

    // abilities persist until an ability command is typed or the turn changes
}

// showPrompt draws the current prompt below the message
void XView::showPrompt(const string &prompt)
{
    lastPrompt = prompt;
    // show the prompt on stdin so the user knows to type
    cout << " " << prompt;
    cout.flush();
}

// readCommand still reads from stdin; X11 is used only for output
string XView::readCommand()
{
    string line;
    if (!getline(cin, line))
    {
        return "quit";
    }

    // trim leading spaces
    size_t first = line.find_first_not_of(" \t");
    if (first != string::npos)
    {
        string cmd = line.substr(first);
        // extract first word
        size_t spacePos = cmd.find(' ');
        string word = cmd.substr(0, spacePos);

        // lowercase the word
        for (char &ch : word)
        {
            // cast to avoid warning with tolower input type
            ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
        }

        // if the command is "ability", clear the overlay to avoid stale data
        if (word == "ability")
        {
            abilitiesActive = false;
            abilitiesOwner = PlayerId::None;
            abilityHeader.clear();
            abilityLinesDrawn = 0;
            for (int i = 0; i < 5; ++i)
            {
                abilityLines[i].clear();
            }
        }
    }

    return line;
}
