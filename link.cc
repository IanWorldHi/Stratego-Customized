export module link;

import types;

using namespace std;

// Link represents a single virus or data piece controlled by a player
export class Link
{
public:
    Link();
    Link(PlayerId owner, LinkKind kind, int strength, char label);

    // getters and setters
    PlayerId getOwner() const;
    LinkKind getKind() const;
    int getStrength() const;
    char getLabel() const;
    bool isAlive() const;
    bool isBoosted() const;
    bool isShielded() const;
    bool isKnownBy(PlayerId player) const;

    void setOwner(PlayerId id);
    void setKind(LinkKind k);
    void setStrength(int s);
    void setLabel(char c);
    void setAlive(bool value);
    void setBoosted(bool value);
    void setShielded(bool value);
    void revealTo(PlayerId player);
    void resetKnowledge();

private:
    PlayerId owner;
    LinkKind kind;
    int strength;
    char label;
    bool alive;
    bool knownByP1;
    bool knownByP2;
    bool boosted;
    bool shielded;
};
