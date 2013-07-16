#include "Swf.hpp"

#include "AudioDecoder.hpp"
#include "AudioEncoder.hpp"
#include "Transcode.hpp"

extern "C"
{
#include <libavcodec/avcodec.h>
}

/*
This code is far from ideal. I was working on a SWF compiler/decompiler called libswfer,
but it's far from complete and I don't currently have time to work on it. libswfer is
able to generate all the bytes for a gaplessly looped MP3 in a SWF, but I don't want to
include libswfer in this project since libswfer is so far from being even half complete.
Instead, I've manually written the bytes here. Once libswfer matures in the potentially
indefinite future, I'll replace all of this messy bit twiddling code.
*/

Swf::Swf()
{
}

Swf::Swf(const std::string& filePath)
{
    open(filePath);
}

Swf::~Swf()
{
    try
    {
        if (file.is_open())
        {
            close();
        }
    }
    catch (...)
    {
    }
}

void Swf::open(const std::string& filePath)
{
    if (file.is_open())
    {
        close();
    }

    // Assumes little endian
    // Also assumes char == 8 bits, short == 16 bits, int == 32 bits

    file.open(filePath, std::ios::binary);
    if (!file)
    {
        throw std::runtime_error("Could not open SWF file for writing");
    }

    char header[] = {
        'F', 'W', 'S', // Non compressed flash file
        10,            // SWF version
        0, 0, 0, 0,    // File length (fill it in later)
        8, 0,          // Frame size (0, 0, 0, 0) (we have no graphics...)
        0, 24,         // FPS (doesn't matter for us; we'll say 24.0)
        0, 0           // Frame count (we have no frames)
    };
    if (!file.write(header, sizeof(header)))
    {
        throw std::runtime_error("Error when writing the SWF's header");
    }
}

void Swf::addSound(AudioDecoder& decoder, AudioEncoder& encoder, const std::string& className, std::function<bool(float)> callback)
{
    if (!file.is_open())
    {
        throw std::logic_error("SWF isn't open for adding a sound");
    }

    if (className.empty())
    {
        throw std::invalid_argument("Class name is empty, but it must not be");
    }

    const std::vector<unsigned char>& data = transcode(decoder, encoder, callback);

    char sr = encoder.getSampleRate() == 5512 ? 0 : // Note: sample rate 5512 is illegal for MP3
        encoder.getSampleRate() == 11025 ? 1 :
        encoder.getSampleRate() == 22050 ? 2 :
        encoder.getSampleRate() == 44100 ? 3 : -1;
    if (sr < 0)
    {
        throw std::invalid_argument("Sample rate is not 5512, 11025, 22050, or 44100 Hz (5512 invalid for MP3)");
    }

    char ss = av_get_bytes_per_sample(encoder.getSampleFormat()) == 1 ? 0 :
        av_get_bytes_per_sample(encoder.getSampleFormat()) == 2 ? 1 : -1;
    if (ss < 0)
    {
        throw std::invalid_argument("Sample size is not 8 or 16 bits");
    }

    char cl = encoder.getChannelCount() == 1 ? 0 :
        encoder.getChannelCount() == 2 ? 1 : -1;
    if (cl < 0)
    {
        throw std::invalid_argument("Channel count is not 1 (mono) or 2 (stereo)");
    }

    // DefineSound tag
    char defineSound[] = {
        (char)((classNames.size() + 1) & 0xff), (char)(((classNames.size() + 1) >> 8) & 0xff),     // ID/tag
        (char)(
        ((encoder.getCodecId() == AV_CODEC_ID_MP3) ? 2 : 3)  << 4 | // MP3 or PCM Little endian
        sr << 2 | // Sample rate
        ss << 1 | // Sample size
        cl)       // Channel layout
    };
    short defineSoundHeader = 14 << 6 | 0x3f;
    int defineSoundSize = sizeof(defineSound) + 4 + data.size() + (encoder.getCodecId() == AV_CODEC_ID_MP3 ? 2 : 0);
    int sampleCount = encoder.getEncodedSampleCount();
    short seekSamples = decoder.getDelay() + encoder.getDelay();
    if (!file.write((const char*)&defineSoundHeader, 2) ||
        !file.write((const char*)&defineSoundSize, 4) ||
        !file.write(defineSound, sizeof(defineSound)) ||
        !file.write((const char*)&sampleCount, 4) ||
        (encoder.getCodecId() == AV_CODEC_ID_MP3 && !file.write((const char*)&seekSamples, 2)) || // This is the key to gapless looping
        !file.write((const char*)data.data(), data.size()))
    {
        throw std::runtime_error("Error when writing the SWF's sound data");
    }

    classNames.push_back(className);
}

void Swf::close()
{
    if (!file.is_open())
    {
        throw std::logic_error("SWF isn't open for closing");
    }

    std::vector<unsigned short> symbolClass(2 + 2 * classNames.size());
    // Symbol count
    symbolClass[0] = classNames.size();

    int symbolClassSize = symbolClass.size() * 2;
    for (std::size_t i = 0; i < classNames.size(); ++i)
    {
        // Symbol ID/tag
        symbolClass[1 + i] = i + 1;
        symbolClassSize += classNames[i].size() + 1;
    }

    // SymbolClass tag
    short symbolClassHeader = (76 << 6) | 0x3f;
    if (!file.write((const char*)&symbolClassHeader, 2) ||
        !file.write((const char*)&symbolClassSize, 4) ||
        !file.write((const char*)symbolClass.data(), symbolClass.size() * 2))
    {
        throw std::runtime_error("Error when writing the SWF's symbol table");
    }

    for (auto& className : classNames)
    {
        if (!file.write(className.c_str(), className.size() + 1))
        {
            throw std::runtime_error("Error when writing the SWF's symbol table entries");
        }
    }

    // End tag
    char end[] = {0, 0};
    if (!file.write(end, sizeof(end)))
    {
        throw std::runtime_error("Error when writing the SWF's end tag");
    }

    // Write the file size back into the header
    int length = file.tellp();
    if (length < 0 ||
        !file.seekp(4) ||
        !file.write((const char*)&length, 4))
    {
        throw std::runtime_error("Error writing the SWF's size");
    }

    file.close();
    file.clear();

    classNames.clear();
}
