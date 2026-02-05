export module view;

import <string>;
import game;
import xwindow;
import types;
import link;

using namespace std;

// IView is the abstract base class for all views
export class IView
{
public:
    virtual ~IView() = default;

    // showBoard prints the full board + player info for the current game state
    virtual void showBoard(const Game &game) = 0;

    // showMessage prints a single line of text
    virtual void showMessage(const string &msg) = 0;

    // showPrompt prints a prompt (without newline) before reading input
    virtual void showPrompt(const string &prompt) = 0;

    // readCommand reads one full command line from the user
    virtual string readCommand() = 0;
};

// TextView prints the game to stdout using plain text
export class TextView : public IView
{
public:
    void showBoard(const Game &game) override;
    void showMessage(const string &msg) override;
    void showPrompt(const string &prompt) override;
    string readCommand() override;
};

// CursesView clears and redraws the board on each update
//  * Needs a char[] buffer to take in text via getnstr from curses.h
export class CursesView : public IView
{
    char inputBuffer[1024];

public:
    CursesView();
    ~CursesView();

    void showBoard(const Game &game) override;
    void showMessage(const string &msg) override;
    void showPrompt(const string &prompt) override;
    string readCommand() override;
};

// XView draws the game using an X11 window via Xwindow
export class XView : public IView
{
    Xwindow window;
    int cellSize;
    int boardOriginX;
    int boardOriginY;
    int statusHeight;
    string lastMessage;
    string lastPrompt;

    // abilities overlay state
    //  * unique to XView to persist ability overlay until player changes
    bool abilitiesActive;
    int abilityLinesDrawn;
    string abilityHeader;
    string abilityLines[5];
    PlayerId abilitiesOwner;
    PlayerId lastBoardPlayer;

public:
    XView();
    ~XView() override = default;

    void showBoard(const Game &game) override;
    void showMessage(const string &msg) override;
    void showPrompt(const string &prompt) override;
    string readCommand() override;

private:
    void clearAll();
    void drawGrid();
    void drawLinks(const Game &game);
    void drawPlayerPanel(const Game &game, PlayerId who, bool top);
    int colourForLink(const Link &link, PlayerId viewer) const;
};
