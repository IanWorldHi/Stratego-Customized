module game;

import <string>;
import <memory>;
import types;
import board;
import link;
import player;
import cli;
import ability;
import errors;

using namespace std;

// helper function creates a concrete Ability for a given code letter
static unique_ptr<Ability> createAbility(char code)
{
    switch (code)
    {
    case 'L':
        return make_unique<LinkBoostAbility>();
    case 'F':
        return make_unique<FirewallAbility>();
    case 'D':
        return make_unique<DownloadAbility>();
    case 'P':
        return make_unique<PolarizeAbility>();
    case 'S':
        return make_unique<ScanAbility>();
    case 'W':
        return make_unique<SwapAbility>();
    case 'J':
        return make_unique<JumpAbility>();
    case 'H':
        return make_unique<ShieldAbility>();
    default:
        throw AbilityError(string("unknown ability code: ") + code);
    }
}

// PlayerAbilities default constructor clears slots and used flags
PlayerAbilities::PlayerAbilities()
{
    for (int i = 0; i < 5; ++i)
    {
        slots[i] = nullptr;
        used[i] = false;
    }
}

// configure builds the slots array from a five-character order string
void PlayerAbilities::configure(const string &order)
{
    if (order.size() != 5)
    {
        throw FatalError("ability order must be exactly 5 characters long");
    }

    owned.clear();
    for (int i = 0; i < 5; ++i)
    {
        slots[i] = nullptr;
        used[i] = false;
    }

    for (int i = 0; i < 5; ++i)
    {
        char c = order[i];
        if (c >= 'a' && c <= 'z')
        {
            c = static_cast<char>(c - 'a' + 'A');
        }

        Ability *ptr = nullptr;

        // try to reuse an existing ability object with the same code
        for (auto &a : owned)
        {
            if (a && a->code() == c)
            {
                ptr = a.get();
                break;
            }
        }

        if (!ptr)
        {
            auto ability = createAbility(c);
            ptr = ability.get();
            owned.push_back(std::move(ability));
        }

        slots[i] = ptr;
        used[i] = false;
    }
}

Ability &PlayerAbilities::abilityAt(int slot)
{
    // precondition 0 <= slot < 5 and slot has an ability
    return *slots[slot];
}

const Ability &PlayerAbilities::abilityAt(int slot) const
{
    // precondition 0 <= slot < 5 and slot has an ability
    return *slots[slot];
}

bool PlayerAbilities::isUsed(int slot) const
{
    // precondition 0 <= slot < 5
    return used[slot];
}

void PlayerAbilities::markUsed(int slot)
{
    // precondition 0 <= slot < 5
    used[slot] = true;
}

int PlayerAbilities::remaining() const
{
    int count = 0;
    for (int i = 0; i < 5; ++i)
    {
        if (slots[i] && !used[i])
        {
            ++count;
        }
    }
    return count;
}

// Game constructor builds the initial board, links, and player states
Game::Game(const CommandLineOptions &options)
    : boardState{},
      current{PlayerId::P1}
{

    players[0] = PlayerState{PlayerId::P1};
    players[1] = PlayerState{PlayerId::P2};

    // configure abilities for each player
    abilities[0].configure(options.ability1);
    abilities[1].configure(options.ability2);

    // no jump/swap queued at the start of the game
    jumpReady[0] = jumpReady[1] = false;
    swapReady[0] = swapReady[1] = false;

    setupServerPorts();

    // link layout for each player is controlled by link1/link2 options
    setupLinksForPlayer(PlayerId::P1, options.link1);
    setupLinksForPlayer(PlayerId::P2, options.link2);
}

// getters and setters

PlayerId Game::currentPlayer() const
{
    return current;
}

const Board &Game::board() const
{
    return boardState;
}

const Link &Game::getLink(int idx) const
{
    // precondition 0 <= idx < 16
    return links[idx];
}

Link &Game::getLink(int idx)
{
    // precondition 0 <= idx < 16
    return links[idx];
}

const PlayerState &Game::getPlayer(PlayerId id) const
{
    int idx = indexFor(id);
    return players[idx];
}

PlayerState &Game::getPlayer(PlayerId id)
{
    int idx = indexFor(id);
    return players[idx];
}

const PlayerAbilities &Game::getAbilities(PlayerId id) const
{
    int idx = indexFor(id);
    return abilities[idx];
}

PlayerAbilities &Game::getAbilities(PlayerId id)
{
    int idx = indexFor(id);
    return abilities[idx];
}

bool Game::isOver() const
{
    return winnerIfAny() != PlayerId::None;
}

// moveLink implements basic movement, capturing, server ports, ability enhanced movement, and off-edge downloads
MoveResult Game::moveLink(char label, Direction dir)
{
    PlayerId mover = current;
    if (mover == PlayerId::None)
    {
        return MoveResult{false, false, PlayerId::None};
    }

    // map label to slot for the current player
    int slot = -1;
    if (mover == PlayerId::P1)
    {
        if (label < 'a' || label > 'h')
        {
            return MoveResult{false, false, PlayerId::None};
        }
        slot = label - 'a';
    }
    else
    { // PlayerId::P2
        if (label < 'A' || label > 'H')
        {
            return MoveResult{false, false, PlayerId::None};
        }
        slot = label - 'A';
    }

    PlayerState &ps = getPlayer(mover);
    int linkIdx = ps.getLinkIndex(slot);
    if (linkIdx < 0)
    {
        // link was already downloaded or does not exist
        return MoveResult{false, false, PlayerId::None};
    }

    Link &piece = links[linkIdx];
    if (!piece.isAlive() || piece.getOwner() != mover)
    {
        // must move one of your own, alive links
        return MoveResult{false, false, PlayerId::None};
    }

    Position src;
    if (!findLinkPosition(linkIdx, src))
    {
        // inconsistent state, treat as invalid move from controller’s perspective
        return MoveResult{false, false, PlayerId::None};
    }

    int dr = 0;
    int dc = 0;

    switch (dir)
    {
    case Direction::Up:
        dr = -1;
        break;
    case Direction::Down:
        dr = 1;
        break;
    case Direction::Left:
        dc = -1;
        break;
    case Direction::Right:
        dc = 1;
        break;
    default:
        return MoveResult{false, false, PlayerId::None};
    }

    if (dr == 0 && dc == 0)
    {
        return MoveResult{false, false, PlayerId::None};
    }

    int moverIdx = indexFor(mover);

    // Jump: if jumpReady is set, allow a two-square move like a temporary boost
    int step = (piece.isBoosted() || jumpReady[moverIdx]) ? 2 : 1;

    Position dest{src.row + dr * step, src.col + dc * step};

    // cannot move off the sides of the board at all
    if (dest.col < 0 || dest.col >= 8)
    {
        return MoveResult{false, false, PlayerId::None};
    }

    PlayerId opponent = (mover == PlayerId::P1) ? PlayerId::P2 : PlayerId::P1;

    // handle moving off the opponent edge for a download
    if (dest.row < 0 || dest.row >= 8)
    {
        bool offOpponentEdge = false;

        if (dc == 0)
        {
            if (mover == PlayerId::P1 &&
                src.row == 7 && dr > 0 && dest.row >= 8)
            {
                offOpponentEdge = true;
            }
            else if (mover == PlayerId::P2 &&
                     src.row == 0 && dr < 0 && dest.row < 0)
            {
                offOpponentEdge = true;
            }
        }

        if (!offOpponentEdge)
        {
            return MoveResult{false, false, PlayerId::None};
        }

        // legal off-edge download of your own link
        applyDownload(linkIdx, mover);

        // Jump/Swap only last until the next successful move
        jumpReady[moverIdx] = false;
        swapReady[moverIdx] = false;

        // switch turn
        current = opponent;
        PlayerId w = winnerIfAny();
        bool over = (w != PlayerId::None);
        return MoveResult{true, over, w};
    }

    // destination is on the board
    Cell &destCell = boardState.at(dest);

    // cannot move onto your own server ports
    if (destCell.isServerPortFor(mover))
    {
        return MoveResult{false, false, PlayerId::None};
    }

    // moving into opponent server port downloads the moving link for the opponent
    if (destCell.isServerPortFor(opponent))
    {
        applyDownload(linkIdx, opponent);

        jumpReady[moverIdx] = false;
        swapReady[moverIdx] = false;

        current = opponent;
        PlayerId w = winnerIfAny();
        bool over = (w != PlayerId::None);
        return MoveResult{true, over, w};
    }

    // if there is a link at the destination, handle blocking, swap, or battle
    if (destCell.getKind() == CellKind::Link)
    {
        int destIdx = destCell.getLinkIndex();
        Link &defender = links[destIdx];

        if (defender.getOwner() == mover)
        {
            // cannot move onto your own link, unless Swap is active
            if (!swapReady[moverIdx])
            {
                return MoveResult{false, false, PlayerId::None};
            }

            // perform a swap between src and dest
            destCell.setLinkIndex(linkIdx);
            boardState.at(src).setLinkIndex(destIdx);

            jumpReady[moverIdx] = false;
            swapReady[moverIdx] = false;

            current = opponent;
            PlayerId w = winnerIfAny();
            bool over = (w != PlayerId::None);
            return MoveResult{true, over, w};
        }

        // reveal both links to both players
        piece.revealTo(PlayerId::P1);
        piece.revealTo(PlayerId::P2);
        defender.revealTo(PlayerId::P1);
        defender.revealTo(PlayerId::P2);

        int atkStrength = piece.getStrength();
        int defStrength = defender.getStrength();

        bool attackerWins = (atkStrength > defStrength) ||
                            (atkStrength == defStrength); // attacker wins ties

        // Shield: if the losing link is shielded, flip the outcome and consume shield
        if (attackerWins)
        {
            // defender would lose
            if (defender.isShielded())
            {
                attackerWins = false;        // flip outcome
                defender.setShielded(false); // consume shield
            }
        }
        else
        {
            // attacker would lose
            if (piece.isShielded())
            {
                attackerWins = true;      // flip outcome
                piece.setShielded(false); // consume shield
            }
        }

        if (attackerWins)
        {
            // attacker downloads the defender
            applyDownload(destIdx, mover);

            // move attacker into destination
            boardState.at(src).clearLink();
            destCell.setLinkIndex(linkIdx);
        }
        else
        {
            // defender downloads the attacker
            applyDownload(linkIdx, opponent);
            // defender stays in place, src cell already cleared by applyDownload
        }

        jumpReady[moverIdx] = false;
        swapReady[moverIdx] = false;

        current = opponent;
        PlayerId w = winnerIfAny();
        bool over = (w != PlayerId::None);
        return MoveResult{true, over, w};
    }

    // firewall on destination square can affect the moving link
    if (destCell.firewallPresent())
    {
        PlayerId fwOwner = destCell.getFirewallOwner();

        if (fwOwner != mover)
        {
            // passing through an opponent firewall reveals this link
            piece.revealTo(PlayerId::P1);
            piece.revealTo(PlayerId::P2);

            if (piece.getKind() == LinkKind::Virus)
            {
                // viruses are immediately downloaded by their owner
                applyDownload(linkIdx, mover);

                jumpReady[moverIdx] = false;
                swapReady[moverIdx] = false;

                current = opponent;
                PlayerId w = winnerIfAny();
                bool over = (w != PlayerId::None);
                return MoveResult{true, over, w};
            }
        }
    }

    // regular move into an empty (or firewalled) square
    boardState.at(src).clearLink();
    destCell.setLinkIndex(linkIdx);

    jumpReady[moverIdx] = false;
    swapReady[moverIdx] = false;

    current = opponent;
    PlayerId w = winnerIfAny();
    bool over = (w != PlayerId::None);
    return MoveResult{true, over, w};
}

// applyDownload updates download counts, reveals the link, and removes it from the board
void Game::applyDownload(int linkIdx, PlayerId receiver)
{
    // applyDownload handles a link being downloaded by a player

    if (linkIdx < 0 || linkIdx >= 16)
    {
        throw FatalError("invalid link index for download");
    }

    if (receiver != PlayerId::P1 && receiver != PlayerId::P2)
    {
        throw FatalError("invalid receiver for download");
    }

    Link &lnk = links[linkIdx];

    // abilities may not download something that has already been downloaded
    if (!lnk.isAlive())
    {
        throw AbilityError("link already downloaded");
    }

    PlayerState &recvState = getPlayer(receiver);

    if (lnk.getKind() == LinkKind::Data)
    {
        recvState.incrDownloadedData();
    }
    else
    {
        recvState.incrDownloadedVirus();
    }

    // reveal to both players, matches battle behaviour and display logic
    lnk.revealTo(PlayerId::P1);
    lnk.revealTo(PlayerId::P2);

    lnk.setAlive(false);

    // remove the link from the board if present
    Position pos;
    if (findLinkPosition(linkIdx, pos))
    {
        boardState.at(pos).clearLink();
    }

    // clear the slot in the owning player's state
    PlayerId owner = lnk.getOwner();
    if (owner == PlayerId::P1 || owner == PlayerId::P2)
    {
        PlayerState &ownerState = getPlayer(owner);
        for (int slot = 0; slot < 8; ++slot)
        {
            if (ownerState.getLinkIndex(slot) == linkIdx)
            {
                ownerState.setLinkIndex(slot, -1);
            }
        }
    }
}

// ability-related behaviour

void Game::applyFirewall(Position pos, PlayerId owner)
{
    // applyFirewall handles placing a firewall on the board for the given player

    if (!boardState.inBounds(pos))
    {
        throw AbilityError("invalid firewall position");
    }

    Cell &cell = boardState.at(pos);

    if (cell.getKind() != CellKind::Empty)
    {
        throw AbilityError("firewall must be placed on an empty square");
    }

    if (cell.isServerPortFor(PlayerId::P1) ||
        cell.isServerPortFor(PlayerId::P2))
    {
        throw AbilityError("cannot place firewall on a server port");
    }

    if (cell.firewallPresent())
    {
        throw AbilityError("square already has a firewall");
    }

    cell.setFirewall(owner);
}

void Game::applyBoost(int linkIdx)
{
    // applyBoost marks the specified link as boosted so it moves two squares

    if (linkIdx < 0 || linkIdx >= 16)
    {
        throw AbilityError("invalid link for boost");
    }

    Link &lnk = links[linkIdx];

    if (!lnk.isAlive())
    {
        throw AbilityError("cannot boost a downloaded link");
    }

    if (lnk.isBoosted())
    {
        throw AbilityError("link is already boosted");
    }

    lnk.setBoosted(true);
}

// applyScan reveals a link to a single player without changing its board state
void Game::applyScan(int linkIdx, PlayerId viewer)
{
    if (linkIdx < 0 || linkIdx >= 16)
    {
        throw FatalError("invalid link index for scan");
    }

    if (viewer != PlayerId::P1 && viewer != PlayerId::P2)
    {
        throw FatalError("invalid viewer for scan");
    }

    Link &lnk = links[linkIdx];

    if (!lnk.isAlive())
    {
        throw AbilityError("cannot scan a downloaded link");
    }

    // only reveal to the player who used Scan
    lnk.revealTo(viewer);
}

void Game::applyPolarize(int linkIdx)
{
    // applyPolarize flips a link between data and virus while keeping strength

    if (linkIdx < 0 || linkIdx >= 16)
    {
        throw AbilityError("invalid link for polarize");
    }

    Link &lnk = links[linkIdx];

    if (!lnk.isAlive())
    {
        throw AbilityError("cannot polarize a downloaded link");
    }

    LinkKind kind = lnk.getKind();

    if (kind == LinkKind::Data)
    {
        lnk.setKind(LinkKind::Virus);
    }
    else if (kind == LinkKind::Virus)
    {
        lnk.setKind(LinkKind::Data);
    }
}

void Game::applyShield(int linkIdx)
{
    // applyShield marks a link so that if it would lose a battle, it wins instead

    if (linkIdx < 0 || linkIdx >= 16)
    {
        throw AbilityError("invalid link for shield");
    }

    Link &lnk = links[linkIdx];

    if (!lnk.isAlive())
    {
        throw AbilityError("cannot shield a downloaded link");
    }

    if (lnk.isShielded())
    {
        throw AbilityError("link already shielded");
    }

    lnk.setShielded(true);
}

void Game::applyJump(PlayerId user)
{
    // applyJump marks that the given player may jump on their next move
    if (user != PlayerId::P1 && user != PlayerId::P2)
    {
        throw FatalError("invalid player for jump");
    }

    int idx = indexFor(user);
    jumpReady[idx] = true;
}

void Game::applySwap(PlayerId user)
{
    // applySwap marks that the given player may swap on their next move
    if (user != PlayerId::P1 && user != PlayerId::P2)
    {
        throw FatalError("invalid player for swap");
    }

    int idx = indexFor(user);
    swapReady[idx] = true;
}

// private helpers

int Game::indexFor(PlayerId id) const
{
    // precondition id is P1 or P2
    if (id == PlayerId::P1)
    {
        return 0;
    }
    return 1;
}

void Game::setupServerPorts()
{
    // middle two squares of first and last row are server ports
    Position p1a{0, 3};
    Position p1b{0, 4};
    Position p2a{7, 3};
    Position p2b{7, 4};

    boardState.at(p1a).setServerPortFor(PlayerId::P1);
    boardState.at(p1b).setServerPortFor(PlayerId::P1);
    boardState.at(p2a).setServerPortFor(PlayerId::P2);
    boardState.at(p2b).setServerPortFor(PlayerId::P2);
}

// setupLinksForPlayer handles creation and placement of links for a player
void Game::setupLinksForPlayer(PlayerId owner, const string &order)
{
    string linkOrder = order;
    if (linkOrder.empty())
    {
        // should not happen because cli provides defaults but keeps Game robust
        linkOrder = "V1V2V3V4D1D2D3D4";
    }

    if (linkOrder.size() != 16)
    {
        throw FatalError("link order must describe exactly 8 links");
    }

    int baseIdx = (owner == PlayerId::P1) ? 0 : 8;
    int startRow = (owner == PlayerId::P1) ? 0 : 7;
    int altRow = (owner == PlayerId::P1) ? 1 : 6;

    PlayerState &ps = players[indexFor(owner)];

    for (int slot = 0; slot < 8; ++slot)
    {
        int pairPos = 2 * slot;
        char kindChar = linkOrder[pairPos];
        char strengthChar = linkOrder[pairPos + 1];

        LinkKind kind = LinkKind::Data;
        if (kindChar == 'V' || kindChar == 'v')
        {
            kind = LinkKind::Virus;
        }
        else if (kindChar == 'D' || kindChar == 'd')
        {
            kind = LinkKind::Data;
        }
        else
        {
            throw FatalError("link type must be V or D");
        }

        int strength = strengthChar - '0';
        if (strength < 1 || strength > 4)
        {
            throw FatalError("link strength must be between 1 and 4");
        }

        char label;
        if (owner == PlayerId::P1)
        {
            label = static_cast<char>('a' + slot);
        }
        else
        {
            label = static_cast<char>('A' + slot);
        }

        int linkIndex = baseIdx + slot;

        links[linkIndex] = Link{owner, kind, strength, label};
        ps.setLinkIndex(slot, linkIndex);

        int col = slot;
        int row = (col == 3 || col == 4) ? altRow : startRow;

        Position pos{row, col};
        Cell &cell = boardState.at(pos);

        // put the link onto the chosen starting square
        cell.setLinkIndex(linkIndex);
    }
}

// findLinkPosition locates the position of a link on the board if it is present
bool Game::findLinkPosition(int linkIdx, Position &pos) const
{
    for (int r = 0; r < 8; ++r)
    {
        for (int c = 0; c < 8; ++c)
        {
            Position p{r, c};
            const Cell &cell = boardState.at(p);
            if (cell.getKind() == CellKind::Link &&
                cell.getLinkIndex() == linkIdx)
            {
                pos = p;
                return true;
            }
        }
    }
    return false;
}

// winnerIfAny checks download counts and returns the winner if someone has won
PlayerId Game::winnerIfAny() const
{
    const PlayerState &p1 = players[0];
    const PlayerState &p2 = players[1];

    // first check virus loss condition
    if (p1.getDownloadedVirus() >= 4)
    {
        return PlayerId::P2;
    }
    if (p2.getDownloadedVirus() >= 4)
    {
        return PlayerId::P1;
    }

    // then check data win condition
    if (p1.getDownloadedData() >= 4)
    {
        return PlayerId::P1;
    }
    if (p2.getDownloadedData() >= 4)
    {
        return PlayerId::P2;
    }

    return PlayerId::None;
}
