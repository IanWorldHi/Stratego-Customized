export module ability;

import <string>;
import types;

using namespace std;

// AbilityContext provides the operations abilities can use on the game state
export class AbilityContext
{
public:
    virtual ~AbilityContext() = default;

    // applyDownload handles a link being downloaded by a player
    virtual void applyDownload(int linkIdx, PlayerId receiver) = 0;

    // applyFirewall handles placing a firewall on the board
    virtual void applyFirewall(Position pos, PlayerId owner) = 0;

    // applyBoost handles applying a link boost effect to a link
    virtual void applyBoost(int linkIdx) = 0;

    // applyScan handles revealing details of a link to a player
    virtual void applyScan(int linkIdx, PlayerId viewer) = 0;

    // applyPolarize handles changing a link between virus and data
    virtual void applyPolarize(int linkIdx) = 0;

    // applyShield marks a link as shielded for combat resolution
    virtual void applyShield(int linkIdx) = 0;

    // applyJump marks that the given player may jump on their next move
    virtual void applyJump(PlayerId user) = 0;

    // applySwap marks that the given player may swap on their next move
    virtual void applySwap(PlayerId user) = 0;
};

// Ability is the abstract base class for all abilities
export class Ability
{
public:
    virtual ~Ability() = default;

    // code returns the single letter that identifies this ability
    virtual char code() const = 0;

    // name returns a human readable name for the ability
    virtual string name() const = 0;

    // use applies the effect of this ability to the game context
    virtual void use(AbilityContext &ctx, PlayerId user, bool hasLabel, bool hasPos, char label, Position pos) = 0;
};

// LinkBoostAbility modifies a link so it moves two squares instead of one
export class LinkBoostAbility : public Ability
{
public:
    char code() const override;
    string name() const override;
    void use(AbilityContext &ctx, PlayerId user, bool hasLabel, bool hasPos, char label, Position pos) override;
};

// FirewallAbility places a firewall onto an empty square
export class FirewallAbility : public Ability
{
public:
    char code() const override;
    string name() const override;
    void use(AbilityContext &ctx, PlayerId user, bool hasLabel, bool hasPos, char label, Position pos) override;
};

// DownloadAbility downloads an opponent link immediately
export class DownloadAbility : public Ability
{
public:
    char code() const override;
    string name() const override;
    void use(AbilityContext &ctx, PlayerId user, bool hasLabel, bool hasPos, char label, Position pos) override;
};

// PolarizeAbility flips a link between data and virus
export class PolarizeAbility : public Ability
{
public:
    char code() const override;
    string name() const override;
    void use(AbilityContext &ctx, PlayerId user, bool hasLabel, bool hasPos, char label, Position pos) override;
};

// ScanAbility reveals the details of a link
export class ScanAbility : public Ability
{
public:
    char code() const override;
    string name() const override;
    void use(AbilityContext &ctx, PlayerId user, bool hasLabel, bool hasPos, char label, Position pos) override;
};

// SwapAbility swaps the positions of two of the current player's links
export class SwapAbility : public Ability
{
public:
    char code() const override;
    string name() const override;
    void use(AbilityContext &ctx, PlayerId user, bool hasLabel, bool hasPos, char label, Position pos) override;
};

// JumpAbility allows a link to move two squares in one move
export class JumpAbility : public Ability
{
public:
    char code() const override;
    string name() const override;
    void use(AbilityContext &ctx, PlayerId user, bool hasLabel, bool hasPos, char label, Position pos) override;
};

// ShieldAbility grants a one time shield to a link
export class ShieldAbility : public Ability
{
public:
    char code() const override;
    string name() const override;
    void use(AbilityContext &ctx, PlayerId user, bool hasLabel, bool hasPos, char label, Position pos) override;
};
