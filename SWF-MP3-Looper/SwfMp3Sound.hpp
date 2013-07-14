#ifndef SWF_SOUND_HPP
#define SWF_SOUND_HPP

#include <string>
#include <vector>

struct SwfMp3Sound
{
    int sampleSize;
    int sampleRate;
    int channelCount;
    int sampleCount;
    bool mp3;
    short seekSamples;
    std::vector<unsigned char> data;

    std::string className;

    void saveSwf(const std::string& path) const;
};

#endif
