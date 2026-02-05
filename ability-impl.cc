module ability;

import <string>;
import types;
import errors;

using namespace std;

// LinkBoostAbility implementation

char LinkBoostAbility::code() const
{
    return 'L';
}

string LinkBoostAbility::name() const
{
    return "Link Boost";
}

void LinkBoostAbility::use(AbilityContext &ctx, PlayerId user, bool hasLabel, bool /*hasPos*/, char label, Position /*pos*/)
{
    // Link Boost requires a specific link label
    if (!hasLabel)
    {
        throw AbilityError("Link Boost requires a link label");
    }

    int slot = -1;
    int baseIdx = 0;

    if (user == PlayerId::P1)
    {
        if (label < 'a' || label > 'h')
        {
            throw AbilityError("invalid link label");
        }
        slot = label - 'a';
        baseIdx = 0; // P1 links are 0..7
    }
    else if (user == PlayerId::P2)
    {
        if (label < 'A' || label > 'H')
        {
            throw AbilityError("invalid link label");
        }
        slot = label - 'A';
        baseIdx = 8; // P2 links are 8..15
    }
    else
    {
        throw AbilityError("invalid player for Link Boost");
    }

    int linkIdx = baseIdx + slot;

    // Game::applyBoost will enforce that the link is alive and valid
    ctx.applyBoost(linkIdx);
}

// FirewallAbility implementation

char FirewallAbility::code() const
{
    return 'F';
}

string FirewallAbility::name() const
{
    return "Firewall";
}

void FirewallAbility::use(AbilityContext &ctx, PlayerId user, bool /*hasLabel*/, bool hasPos, char /*label*/, Position pos)
{
    if (!hasPos)
    {
        throw AbilityError("Firewall requires a board position");
    }

    // basic bounds check; the game will double-check and enforce rules
    if (pos.row < 0 || pos.row >= 8 ||
        pos.col < 0 || pos.col >= 8)
    {
        throw AbilityError("invalid firewall position");
    }

    ctx.applyFirewall(pos, user);
}

// DownloadAbility implementation

char DownloadAbility::code() const
{
    return 'D';
}

string DownloadAbility::name() const
{
    return "Download";
}

// Download immediately downloads an opponent link by label
void DownloadAbility::use(AbilityContext &ctx, PlayerId user, bool hasLabel, bool hasPos, char label, Position /*pos*/)
{
    // Download requires exactly a single link label
    if (!hasLabel || hasPos)
    {
        throw AbilityError("Download requires a single link label");
    }

    // figure out which player owns the target label and which slot it is
    PlayerId targetOwner;
    int slot = -1;

    if (label >= 'a' && label <= 'h')
    {
        targetOwner = PlayerId::P1;
        slot = label - 'a';
    }
    else if (label >= 'A' && label <= 'H')
    {
        targetOwner = PlayerId::P2;
        slot = label - 'A';
    }
    else
    {
        throw AbilityError("invalid link label");
    }

    if (user != PlayerId::P1 && user != PlayerId::P2)
    {
        throw AbilityError("invalid player for Download");
    }

    // must target an opponent link
    if (targetOwner == user)
    {
        throw AbilityError("Download must target an opponent link");
    }

    int baseIdx = (targetOwner == PlayerId::P1) ? 0 : 8;
    int linkIdx = baseIdx + slot;

    // delegate to the model to actually download the link
    ctx.applyDownload(linkIdx, user);
}

// PolarizeAbility implementation

char PolarizeAbility::code() const
{
    return 'P';
}

string PolarizeAbility::name() const
{
    return "Polarize";
}

// Polarize changes a link from data <-> virus while keeping the same strength
void PolarizeAbility::use(AbilityContext &ctx, PlayerId /*user*/, bool hasLabel, bool hasPos, char label, Position /*pos*/)
{
    // Polarize is targeted by link label only
    if (!hasLabel || hasPos)
    {
        throw AbilityError("Polarize requires a link label");
    }

    PlayerId targetOwner;
    int slot = -1;

    if (label >= 'a' && label <= 'h')
    {
        targetOwner = PlayerId::P1;
        slot = label - 'a';
    }
    else if (label >= 'A' && label <= 'H')
    {
        targetOwner = PlayerId::P2;
        slot = label - 'A';
    }
    else
    {
        throw AbilityError("invalid link label");
    }

    int baseIdx = (targetOwner == PlayerId::P1) ? 0 : 8;
    int linkIdx = baseIdx + slot;

    // model enforces that the link is alive
    ctx.applyPolarize(linkIdx);
}

// ScanAbility implementation

char ScanAbility::code() const
{
    return 'S';
}

string ScanAbility::name() const
{
    return "Scan";
}

// Scan reveals the type and strength of any link on the field to the user
void ScanAbility::use(AbilityContext &ctx, PlayerId user, bool hasLabel, bool hasPos, char label, Position /*pos*/)
{
    // We support Scan by label only
    if (!hasLabel || hasPos)
    {
        throw AbilityError("Scan requires a link label");
    }

    PlayerId targetOwner;
    int slot = -1;

    if (label >= 'a' && label <= 'h')
    {
        targetOwner = PlayerId::P1;
        slot = label - 'a';
    }
    else if (label >= 'A' && label <= 'H')
    {
        targetOwner = PlayerId::P2;
        slot = label - 'A';
    }
    else
    {
        throw AbilityError("invalid link label");
    }

    int baseIdx = (targetOwner == PlayerId::P1) ? 0 : 8;
    int linkIdx = baseIdx + slot;

    // model will reveal this link to the viewing player only
    ctx.applyScan(linkIdx, user);
}

// SwapAbility implementation

char SwapAbility::code() const
{
    return 'W';
}

string SwapAbility::name() const
{
    return "Swap";
}

// Swap lets the user swap any moving link with a friendly link on the next move
void SwapAbility::use(AbilityContext &ctx, PlayerId user, bool hasLabel, bool hasPos, char /*label*/, Position /*pos*/)
{
    // Swap is a one-turn global effect and does not take a target
    if (hasLabel || hasPos)
    {
        throw AbilityError("Swap does not take a target");
    }

    ctx.applySwap(user);
}

// JumpAbility implementation

char JumpAbility::code() const
{
    return 'J';
}

string JumpAbility::name() const
{
    return "Jump";
}

// Jump lets the user move one of their links two squares on their next move
void JumpAbility::use(AbilityContext &ctx, PlayerId user, bool hasLabel, bool hasPos, char /*label*/, Position /*pos*/)
{
    // Jump is a one-turn global effect and does not take a target
    if (hasLabel || hasPos)
    {
        throw AbilityError("Jump does not take a target");
    }

    ctx.applyJump(user);
}

// ShieldAbility implementation

char ShieldAbility::code() const
{
    return 'H';
}

string ShieldAbility::name() const
{
    return "Shield";
}

// Shield grants a one time shield to one of the user's own links
void ShieldAbility::use(AbilityContext &ctx, PlayerId user, bool hasLabel, bool hasPos, char label, Position /*pos*/)
{
    // Shield is targeted by label only
    if (!hasLabel || hasPos)
    {
        throw AbilityError("Shield requires a link label");
    }

    if (user != PlayerId::P1 && user != PlayerId::P2)
    {
        throw AbilityError("invalid player for Shield");
    }

    PlayerId targetOwner;
    int slot = -1;

    if (label >= 'a' && label <= 'h')
    {
        targetOwner = PlayerId::P1;
        slot = label - 'a';
    }
    else if (label >= 'A' && label <= 'H')
    {
        targetOwner = PlayerId::P2;
        slot = label - 'A';
    }
    else
    {
        throw AbilityError("invalid link label");
    }

    // you can only shield your own link
    if (targetOwner != user)
    {
        throw AbilityError("Shield must target one of your own links");
    }

    int baseIdx = (user == PlayerId::P1) ? 0 : 8;
    int linkIdx = baseIdx + slot;

    ctx.applyShield(linkIdx);
}
