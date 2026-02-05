module cli;

import <map>;
import <string>;
import errors;

using namespace std;

// CommandLineOptions default constructor sets defaults
CommandLineOptions::CommandLineOptions() : ability1{"LFDSP"},
                                           ability2{"LFDSP"},
                                           link1{"V1V2V3V4D1D2D3D4"},
                                           link2{"V1V2V3V4D1D2D3D4"},
                                           enableBonus{false},
                                           enableGraphics{false} {}

// validateAbilityString checks that an ability string is exactly 5 chars,
// uses only known ability codes, and has at most 2 of each kind
static void validateAbilityString(const std::string &s, const std::string &optName)
{
    if (s.size() != 5)
    {
        throw ParseError(optName + " must be exactly 5 characters");
    }

    std::map<char, int> counts;

    for (char ch : s)
    {
        switch (ch)
        {
        case 'L': // Link Boost
        case 'F': // Firewall
        case 'D': // Download
        case 'S': // Scan
        case 'P': // Polarize
        case 'W': // Swap
        case 'J': // Jump
        case 'H': // Shield
            counts[ch] += 1;
            if (counts[ch] > 2)
            {
                std::string msg = "at most 2 of each ability are allowed in ";
                throw ParseError(msg + optName);
            }
            break;
        default:
        {
            std::string m = "invalid ability '";
            m += ch;
            m += "' in ";
            m += optName;
            throw ParseError(m);
        }
        }
    }
}

// parseOptions handles -ability1/-ability2, -link1/-link2, and view flags
CommandLineOptions parseOptions(int argc, char *argv[])
{
    CommandLineOptions opts;

    for (int i = 1; i < argc; ++i)
    {
        string arg = argv[i];

        if (arg == "-ability1")
        {
            if (i + 1 >= argc)
            {
                throw ParseError("missing ability1 string");
            }
            std::string val = argv[++i];
            validateAbilityString(val, "ability1");
            opts.ability1 = val;
        }
        else if (arg == "-ability2")
        {
            if (i + 1 >= argc)
            {
                throw ParseError("missing ability2 string");
            }
            std::string val = argv[++i];
            validateAbilityString(val, "ability2");
            opts.ability2 = val;
        }
        else if (arg == "-link1")
        {
            if (i + 1 >= argc)
            {
                throw ParseError("missing argument for -link1");
            }
            opts.link1 = argv[++i];
            if (opts.link1.size() != 16)
            {
                throw ParseError("link1 must describe 8 links like V1V2V3V4D1D2D3D4");
            }
        }
        else if (arg == "-link2")
        {
            if (i + 1 >= argc)
            {
                throw ParseError("missing argument for -link2");
            }
            opts.link2 = argv[++i];
            if (opts.link2.size() != 16)
            {
                throw ParseError("link2 must describe 8 links like V1V2V3V4D1D2D3D4");
            }
        }
        else if (arg == "-enableBonus" || arg == "-enablebonus")
        {
            // cannot mix curses and graphics flags
            if (opts.enableGraphics)
            {
                throw ParseError("cannot combine -graphics with -enableBonus/-enablebonus");
            }
            opts.enableBonus = true;
        }
        else if (arg == "-graphics")
        {
            // cannot mix graphics and curses flags
            if (opts.enableBonus)
            {
                throw ParseError("cannot combine -graphics with -enableBonus/-enablebonus");
            }
            opts.enableGraphics = true;
        }
        else
        {
            // unknown option is treated as a parse error
            throw ParseError("unknown option: " + arg);
        }
    }

    return opts;
}

