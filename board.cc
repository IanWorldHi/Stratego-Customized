export module board;

import types;

using namespace std;

// Cell stores the contents and flags for a single board square
export class Cell
{
public:
    Cell();

    // getters and setters
    CellKind getKind() const;
    int getLinkIndex() const;
    bool firewallPresent() const;
    PlayerId getFirewallOwner() const;
    bool isServerPortFor(PlayerId player) const;

    void setKind(CellKind kindValue);
    void setLinkIndex(int index);
    void clearLink();
    void setFirewall(PlayerId owner);
    void clearFirewall();
    void setServerPortFor(PlayerId player);

private:
    CellKind kind;
    int linkIndex;
    bool hasFirewall;
    PlayerId firewallOwner;
    bool serverPortP1;
    bool serverPortP2;
};

// Board is the 8x8 grid of Cells used in the game
export class Board
{
public:
    Board();

    // getters and setters
    bool inBounds(Position p) const;
    Cell &at(Position p);
    const Cell &at(Position p) const;

private:
    Cell cells[8][8];
};
