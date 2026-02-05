module;

#include <unistd.h>

module controller;

import <string>;
import <sstream>;
import <fstream>;
import <iostream>;
import game;
import view;
import ability;
import types;
import errors;

using namespace std;

// Controller constructor initializes references and state
Controller::Controller(Game &g, IView &v) : game{g},
                                            view{v},
                                            abilityUsedThisTurn{false},
                                            quitRequested{false},
                                            lastMessage{"P1's Turn"},
                                            suppressBoardOnce{false} {}

// currentPrompt handles building the right prompt for the active player
string Controller::currentPrompt() const
{
    PlayerId curr = game.currentPlayer();
    if (curr == PlayerId::P1)
    {
        return "P1 > ";
    }
    else if (curr == PlayerId::P2)
    {
        return "P2 > ";
    }
    return "> ";
}

// runs the main loop: display, read command, execute, handle errors
void Controller::run()
{
    while (!quitRequested && !game.isOver())
    {
        if (!suppressBoardOnce)
        {
            view.showBoard(game);
        }
        else
        {
            // keep current screen (for ability list) and just add msg + prompt
            suppressBoardOnce = false;
        }

        view.showMessage("msg: " + lastMessage);
        view.showPrompt(currentPrompt());

        string line = view.readCommand();
        // ignore pure whitespace lines
        {
            bool onlySpace = true;
            for (char ch : line)
            {
                if (!isspace(static_cast<unsigned char>(ch)))
                {
                    onlySpace = false;
                    break;
                }
            }
            if (onlySpace)
            {
                continue;
            }
        }

        try
        {
            executeCommand(line);
        }
        catch (const RaiiError &err)
        {
            string base = err.message();
            PlayerId curr = game.currentPlayer();
            if (curr == PlayerId::P1)
            {
                lastMessage = base + ", P1's Turn";
            }
            else if (curr == PlayerId::P2)
            {
                lastMessage = base + ", P2's Turn";
            }
            else
            {
                lastMessage = base;
            }
        }
    }

    // final display if game ended via moves or abilities
    if (game.isOver())
    {
        view.showBoard(game);
        view.showMessage("msg: " + lastMessage);
        usleep(2000000); // give time to see the game ended
    }
}

// executeCommand parses the first word and dispatches to the right handler
void Controller::executeCommand(string line)
{
    istringstream iss{line};
    string cmd;
    if (!(iss >> cmd))
    {
        return; // empty line
    }

    if (cmd == "move")
    {
        cmdMove(iss);
    }
    else if (cmd == "ability")
    {
        cmdAbility(iss);
    }
    else if (cmd == "abilities")
    {
        cmdAbilities();
    }
    else if (cmd == "board")
    {
        cmdBoard();
    }
    else if (cmd == "sequence")
    {
        string fileName;
        if (!(iss >> fileName))
        {
            throw ParseError("missing filename for sequence");
        }
        cmdSequence(fileName);
    }
    else if (cmd == "quit")
    {
        quitRequested = true;
    }
    else
    {
        throw ParseError("unknown command: " + cmd);
    }
}

// cmdMove parses label and direction, validates, and calls Game::moveLink
void Controller::cmdMove(istream &iss)
{
    char label;
    string dirStr;

    if (!(iss >> label >> dirStr))
    {
        throw ParseError("usage: move <label> <dir>");
    }

    Direction dir;

    if (dirStr == "up")
    {
        dir = Direction::Up;
    }
    else if (dirStr == "down")
    {
        dir = Direction::Down;
    }
    else if (dirStr == "left")
    {
        dir = Direction::Left;
    }
    else if (dirStr == "right")
    {
        dir = Direction::Right;
    }
    else
    {
        throw ParseError("invalid direction: " + dirStr);
    }

    MoveResult res = game.moveLink(label, dir);
    if (!res.ok)
    {
        // model rejected the move as illegal
        throw MoveError("Invalid Move");
    }

    if (res.gameOver)
    {
        if (res.winner == PlayerId::P1)
        {
            lastMessage = "Player 1 wins";
        }
        else if (res.winner == PlayerId::P2)
        {
            lastMessage = "Player 2 wins";
        }
        else
        {
            lastMessage = "Game over";
        }
        quitRequested = true;
    }
    else
    {
        // successful move -> next player's turn
        PlayerId curr = game.currentPlayer();
        if (curr == PlayerId::P1)
        {
            lastMessage = "P1's Turn";
        }
        else if (curr == PlayerId::P2)
        {
            lastMessage = "P2's Turn";
        }
        else
        {
            lastMessage.clear();
        }
        // new turn starts with no ability used yet
        abilityUsedThisTurn = false;
    }
}

// cmdAbility parses ability index and optional args, then calls Ability::use
void Controller::cmdAbility(istream &iss)
{
    int id;
    if (!(iss >> id))
    {
        throw ParseError("usage: ability <N> [args]");
    }

    if (id < 1 || id > 5)
    {
        throw ParseError("ability id must be between 1 and 5");
    }

    if (abilityUsedThisTurn)
    {
        throw AbilityError("ability already used this turn");
    }

    PlayerId user = game.currentPlayer();
    PlayerAbilities &pa = game.getAbilities(user);

    int slot = id - 1;
    if (pa.isUsed(slot))
    {
        throw AbilityError("ability card already used");
    }

    Ability &ability = pa.abilityAt(slot);

    // parse optional arguments: either a link label, a position, or both
    bool hasLabel = false;
    bool hasPos = false;
    char label = '?';
    Position pos{0, 0};

    string token;
    if (iss >> token)
    {
        // first extra token could be a link label or a row number
        if (token.size() == 1 &&
            ((token[0] >= 'a' && token[0] <= 'h') ||
             (token[0] >= 'A' && token[0] <= 'H')))
        {
            hasLabel = true;
            label = token[0];

            // optionally parse row/col after label
            int r, c;
            if (iss >> r >> c)
            {
                hasPos = true;
                pos = Position{r, c};
            }
        }
        else
        {
            // treat it as the row
            int r;
            if (!(istringstream{token} >> r))
            {
                throw ParseError("invalid ability target");
            }

            int c;
            if (!(iss >> c))
            {
                throw ParseError("missing column for ability");
            }

            hasPos = true;
            pos = Position{r, c};
        }
    }

    // Ability::use is currently a stub that throws AbilityError
    // Later we will implement real behavior in each concrete ability
    ability.use(game, user, hasLabel, hasPos, label, pos);

    pa.markUsed(slot);
    abilityUsedThisTurn = true;

    // keep the same player's turn; movement will still be required
    PlayerId curr = game.currentPlayer();
    if (curr == PlayerId::P1)
    {
        lastMessage = "P1's Turn";
    }
    else if (curr == PlayerId::P2)
    {
        lastMessage = "P2's Turn";
    }
}

// cmdBoard just redraws the board on demand
void Controller::cmdBoard()
{
    view.showBoard(game);
}

// cmdAbilities shows the current player's abilities and usage status
void Controller::cmdAbilities()
{
    PlayerId user = game.currentPlayer();
    const PlayerAbilities &pa = game.getAbilities(user);

    view.showMessage("Abilities:");

    for (int i = 0; i < 5; ++i)
    {
        const Ability &a = pa.abilityAt(i);
        bool used = pa.isUsed(i);

        ostringstream oss;
        oss << (i + 1) << ": " << a.code()
            << " (" << a.name() << ") "
            << (used ? "[used]" : "[ready]");
        view.showMessage(oss.str());
    }

    // keep the abilities list on screen; skip next board redraw
    suppressBoardOnce = true;
}

// cmdSequence opens a file and feeds each line back into executeCommand
void Controller::cmdSequence(string file)
{
    ifstream in{file};
    if (!in)
    {
        throw ParseError("could not open sequence file: " + file);
    }

    string line;
    while (getline(in, line))
    {
        // stop processing sequence file if the game has ended or quit was requested
        if (quitRequested || game.isOver())
        {
            break;
        }

        // ignore empty lines in the sequence file
        bool onlySpace = true;
        for (char ch : line)
        {
            if (!isspace(static_cast<unsigned char>(ch)))
            {
                onlySpace = false;
                break;
            }
        }
        if (onlySpace)
        {
            continue;
        }

        executeCommand(line);
    }
}
