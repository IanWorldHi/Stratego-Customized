export module types;

using namespace std;

// identifies which player a piece or event belongs to
export enum class PlayerId {
    P1,
    P2,
    None
};

// differentiates between data and virus links
export enum class LinkKind {
    Data,
    Virus
};

// describes the content of a board cell
export enum class CellKind {
    Empty,
    Link
};

// movement directions for links on the board
export enum class Direction {
    Up,
    Down,
    Left,
    Right
};

// row and column coordinates on the board
export struct Position
{
    int row;
    int col;
};

// MoveResult summarizes the outcome of a moveLink call
export struct MoveResult
{
    bool ok;
    bool gameOver;
    PlayerId winner;

    MoveResult(bool ok = false, bool gameOver = false, PlayerId winner = PlayerId::None);
};
