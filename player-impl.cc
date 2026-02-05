module player;

import types;

using namespace std;

// PlayerState default constructor builds an empty player with no links
PlayerState::PlayerState() : id{PlayerId::None},
                             downloadedData{0},
                             downloadedVirus{0}
{
    for (int i = 0; i < 8; ++i)
    {
        linkIndices[i] = -1;
    }
}

// PlayerState constructor with explicit id
PlayerState::PlayerState(PlayerId idValue) : id{idValue},
                                             downloadedData{0},
                                             downloadedVirus{0}
{
    for (int i = 0; i < 8; ++i)
    {
        linkIndices[i] = -1;
    }
}

// getters and setters

PlayerId PlayerState::getId() const
{
    return id;
}

void PlayerState::setId(PlayerId idValue)
{
    id = idValue;
}

int PlayerState::getDownloadedData() const
{
    return downloadedData;
}

int PlayerState::getDownloadedVirus() const
{
    return downloadedVirus;
}

void PlayerState::incrDownloadedData()
{
    ++downloadedData;
}

void PlayerState::incrDownloadedVirus()
{
    ++downloadedVirus;
}

int PlayerState::getLinkIndex(int slot) const
{
    // precondition 0 <= slot < 8
    return linkIndices[slot];
}

void PlayerState::setLinkIndex(int slot, int index)
{
    // precondition 0 <= slot < 8
    linkIndices[slot] = index;
}

int PlayerState::totalDownloaded() const
{
    return downloadedData + downloadedVirus;
}
