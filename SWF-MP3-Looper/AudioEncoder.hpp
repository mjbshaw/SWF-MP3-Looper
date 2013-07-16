#ifndef AUDIO_ENCODER_HPP
#define AUDIO_ENCODER_HPP

extern "C"
{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/audio_fifo.h>
}

#include <cstdint>
#include <memory>
#include <functional>
#include <vector>

class AudioEncoder
{
public:
    AudioEncoder(const AudioEncoder&) = delete;
    AudioEncoder& operator= (const AudioEncoder&) = delete;

    AudioEncoder(AVCodecID codecId, int channelCount, int sampleRate, int audioQuality, int vbrQuality);

    int getChannelCount() const;
    uint64_t getChannelLayout() const;
    AVSampleFormat getSampleFormat() const;
    int getSampleRate() const;
    int getDelay() const;
    int getEncodedSampleCount() const;
    bool isMp3() const;
    int getSampleSize() const;

    void processSamples(const unsigned char** buffer, int sampleCount);

    const std::vector<unsigned char>& getEncodedData() const;

private:
    int encodedSampleCount;

    std::vector<unsigned char> encodedData;

    // Order of these members is important (so that they are destroyed in the right order)
    std::unique_ptr<AVCodecContext, std::function<void(AVCodecContext*)>> context;
    std::unique_ptr<AVAudioFifo, std::function<void(AVAudioFifo*)>> fifo;
    std::unique_ptr<unsigned char[]> frameBuffer;
    std::unique_ptr<AVFrame, std::function<void(void*)>> frame;

    AVCodec* codec;
};

#endif
