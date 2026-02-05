export module controller;

import <string>;
import <iostream>;
import game;
import view;

using namespace std;

// Controller coordinates commands between the view and the game
export class Controller
{
public:
    Controller(Game &game, IView &view);

    // runs the main game loop until quit, EOF, or game over
    void run();

    // executeCommand parses a single command line and dispatches helpers
    void executeCommand(string line);

    // cmdMove parses and executes a move command
    void cmdMove(istream &iss);

    // cmdAbility parses and executes an ability command
    void cmdAbility(istream &iss);

    // cmdBoard shows the current board
    void cmdBoard();

    // cmdAbilities lists the current player's ability cards
    void cmdAbilities();

    // cmdSequence executes commands from a file
    void cmdSequence(string file);

private:
    Game &game;
    IView &view;
    bool abilityUsedThisTurn;
    bool quitRequested;
    string lastMessage;
    bool suppressBoardOnce;

    // currentPrompt builds the prompt string for the active player
    string currentPrompt() const;
};
