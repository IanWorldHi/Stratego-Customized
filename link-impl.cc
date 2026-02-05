module link;

import types;

using namespace std;

// Link default constructor builds a neutral & dead placeholder link
//  * exists to make compiler happy
Link::Link() : owner{PlayerId::None},
               kind{LinkKind::Data},
               strength{0},
               label{'?'},
               alive{false},
               knownByP1{false},
               knownByP2{false},
               boosted{false},
               shielded{false} {}

// Link main constructor records basic identity of the piece
Link::Link(PlayerId ownerValue, LinkKind kindValue, int strengthValue, char labelValue) : owner{ownerValue},
                                                                                          kind{kindValue},
                                                                                          strength{strengthValue},
                                                                                          label{labelValue},
                                                                                          alive{true},
                                                                                          knownByP1{false},
                                                                                          knownByP2{false},
                                                                                          boosted{false},
                                                                                          shielded{false} {}

// getters and setters

PlayerId Link::getOwner() const
{
    return owner;
}

LinkKind Link::getKind() const
{
    return kind;
}

int Link::getStrength() const
{
    return strength;
}

char Link::getLabel() const
{
    return label;
}

bool Link::isAlive() const
{
    return alive;
}

bool Link::isBoosted() const
{
    return boosted;
}

bool Link::isShielded() const
{
    return shielded;
}

bool Link::isKnownBy(PlayerId player) const
{
    if (player == PlayerId::P1)
    {
        return knownByP1;
    }
    else if (player == PlayerId::P2)
    {
        return knownByP2;
    }
    return false;
}

void Link::setOwner(PlayerId id)
{
    owner = id;
}

void Link::setKind(LinkKind k)
{
    kind = k;
}

void Link::setStrength(int s)
{
    strength = s;
}

void Link::setLabel(char c)
{
    label = c;
}

void Link::setAlive(bool value)
{
    alive = value;
}

void Link::setBoosted(bool value)
{
    boosted = value;
}

void Link::setShielded(bool value)
{
    shielded = value;
}

void Link::revealTo(PlayerId player)
{
    if (player == PlayerId::P1)
    {
        knownByP1 = true;
    }
    else if (player == PlayerId::P2)
    {
        knownByP2 = true;
    }
}

void Link::resetKnowledge()
{
    knownByP1 = false;
    knownByP2 = false;
}
