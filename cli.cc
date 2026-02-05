export module cli;

import <string>;

using namespace std;

// CommandLineOptions stores configuration parsed from argv
export struct CommandLineOptions
{
    string ability1;
    string ability2;
    string link1;
    string link2;
    bool enableBonus;     // use CursesView when true
    bool enableGraphics;  // use XView when true

    CommandLineOptions();
};

// parseOptions reads argv and fills a CommandLineOptions instance
export CommandLineOptions parseOptions(int argc, char *argv[]);

