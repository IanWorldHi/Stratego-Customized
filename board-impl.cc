module board;

import types;

using namespace std;

// Cell default constructor starts empty with no flags
Cell::Cell() : kind{CellKind::Empty},
               linkIndex{-1},
               hasFirewall{false},
               firewallOwner{PlayerId::None},
               serverPortP1{false},
               serverPortP2{false} {}

// getters and setters for Cell

CellKind Cell::getKind() const
{
    return kind;
}

int Cell::getLinkIndex() const
{
    return linkIndex;
}

bool Cell::firewallPresent() const
{
    return hasFirewall;
}

PlayerId Cell::getFirewallOwner() const
{
    return firewallOwner;
}

bool Cell::isServerPortFor(PlayerId player) const
{
    if (player == PlayerId::P1)
    {
        return serverPortP1;
    }
    else if (player == PlayerId::P2)
    {
        return serverPortP2;
    }
    return false;
}

void Cell::setKind(CellKind kindValue)
{
    kind = kindValue;
}

void Cell::setLinkIndex(int index)
{
    linkIndex = index;
    kind = (index >= 0) ? CellKind::Link : CellKind::Empty;
}

void Cell::clearLink()
{
    linkIndex = -1;
    kind = CellKind::Empty;
}

void Cell::setFirewall(PlayerId owner)
{
    hasFirewall = true;
    firewallOwner = owner;
}

void Cell::clearFirewall()
{
    hasFirewall = false;
    firewallOwner = PlayerId::None;
}

void Cell::setServerPortFor(PlayerId player)
{
    serverPortP1 = (player == PlayerId::P1);
    serverPortP2 = (player == PlayerId::P2);
}

// Board implementation

// Board constructor clears all cells to their default state
Board::Board()
{
    for (int r = 0; r < 8; ++r)
    {
        for (int c = 0; c < 8; ++c)
        {
            cells[r][c] = Cell(); // default cell
        }
    }
}

bool Board::inBounds(Position p) const
{
    return p.row >= 0 && p.row < 8 &&
           p.col >= 0 && p.col < 8;
}

Cell &Board::at(Position p)
{
    // precondition p is in bounds for the board
    return cells[p.row][p.col];
}

const Cell &Board::at(Position p) const
{
    // precondition p is in bounds for the board
    return cells[p.row][p.col];
}
