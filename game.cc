export module game;

import <string>;
import <vector>;
import <memory>;
import types;
import board;
import link;
import player;
import cli;
import ability;

using namespace std;

// PlayerAbilities stores the five ability cards for a player
export class PlayerAbilities
{
public:
    PlayerAbilities();

    // configure builds ability slots from a 5-character string of codes
    void configure(const string &order);

    // getters and setters
    Ability &abilityAt(int slot);
    const Ability &abilityAt(int slot) const;
    bool isUsed(int slot) const;
    void markUsed(int slot);
    int remaining() const;

private:
    vector<unique_ptr<Ability>> owned; // ensures the memory is cleared when out of scope
    Ability *slots[5];                 // store the raw pointers to avoid needing to .get()
    bool used[5];
};

// Game owns the board, links, and player states for a single match
export class Game : public AbilityContext
{
public:
    Game(const CommandLineOptions &options);

    // getters and setters
    PlayerId currentPlayer() const;
    const Board &board() const;
    const Link &getLink(int idx) const;
    Link &getLink(int idx);
    const PlayerState &getPlayer(PlayerId id) const;
    PlayerState &getPlayer(PlayerId id);
    const PlayerAbilities &getAbilities(PlayerId id) const;
    PlayerAbilities &getAbilities(PlayerId id);

    bool isOver() const;

    // moveLink implements movement and capturing rules
    MoveResult moveLink(char label, Direction dir);

    // applyDownload handles a link being downloaded by a player
    void applyDownload(int linkIdx, PlayerId receiver) override;

    // applyFirewall handles placing a firewall on the board
    void applyFirewall(Position pos, PlayerId owner) override;

    // applyBoost handles applying a link boost effect to a link
    void applyBoost(int linkIdx) override;

    // applyScan handles revealing details of a link to a player
    void applyScan(int linkIdx, PlayerId viewer) override;

    // applyPolarize handles changing a link between virus and data
    void applyPolarize(int linkIdx) override;

    // applyShield handles applying a shied to a link
    void applyShield(int linkIdx) override;

    // applyJump marks that the given player may jump on their next move
    void applyJump(PlayerId user) override;

    // applySwap marks that the given player may swap on their next move
    void applySwap(PlayerId user) override;

private:
    Board boardState;
    Link links[16];
    PlayerState players[2];
    PlayerAbilities abilities[2];
    PlayerId current;

    // temporary per-player flags for one-turn ability effects
    bool jumpReady[2];
    bool swapReady[2];

    // indexFor converts a PlayerId into an index into players
    int indexFor(PlayerId id) const;

    // setupServerPorts marks the server port cells at the board edges
    void setupServerPorts();

    // setupLinksForPlayer builds links and places them on the board
    void setupLinksForPlayer(PlayerId owner, const string &order);

    // findLinkPosition locates the current board position of a link
    bool findLinkPosition(int linkIdx, Position &pos) const;

    // winnerIfAny checks download totals and returns the winner or PlayerId::None
    PlayerId winnerIfAny() const;
};
