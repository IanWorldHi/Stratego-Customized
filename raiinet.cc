import <iostream>;
import <exception>;

import cli;
import game;
import view;
import controller;
import errors;

using namespace std;

// main builds the game, chooses a view, and hands control to the controller
int main(int argc, char *argv[])
{
    try
    {
        // parseOptions reads -ability1/-ability2/-link1/-link2 and view flags
        CommandLineOptions options = parseOptions(argc, argv);

        // Game holds all model state for a single match
        Game game{options};

        if (options.enableGraphics)
        {
            // use X11 view
            XView xView;
            Controller controller{game, xView};
            controller.run();
        }
        else if (options.enableBonus)
        {
            // use ncurses view
            CursesView cursesView;
            Controller controller{game, cursesView};
            controller.run();
        }
        else
        {
            // plain text view
            TextView textView;
            Controller controller{game, textView};
            controller.run();
        }
    }
    catch (const ParseError &e)
    {
        cerr << "Command line error: " << e.message() << endl;
        return 1;
    }
    catch (const RaiiError &e)
    {
        cerr << "RAIInet error: " << e.message() << endl;
        return 1;
    }
    catch (const exception &e)
    {
        cerr << "Unexpected standard exception: " << e.what() << endl;
        return 1;
    }
    catch (...)
    {
        cerr << "Unknown fatal error" << endl;
        return 1;
    }

    return 0;
}

