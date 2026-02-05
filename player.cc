export module player;

import types;

using namespace std;

// PlayerState tracks per player links and download totals
export class PlayerState
{
public:
    PlayerState();
    explicit PlayerState(PlayerId idValue);

    // getters and setters
    PlayerId getId() const;
    void setId(PlayerId idValue);

    int getDownloadedData() const;
    int getDownloadedVirus() const;
    void incrDownloadedData();
    void incrDownloadedVirus();

    int getLinkIndex(int slot) const;
    void setLinkIndex(int slot, int index);

    int totalDownloaded() const;

private:
    PlayerId id;
    int downloadedData;
    int downloadedVirus;
    int linkIndices[8];
};
