#include "Transcode.hpp"
#include "AudioDecoder.hpp"
#include "AudioEncoder.hpp"

extern "C"
{
#include <libswresample/swresample.h>
}

#include <stdexcept>

std::vector<unsigned char> transcode(AudioDecoder& decoder, AudioEncoder& encoder, std::function<bool(float)> callback)
{
    SwrContext* swrTemp = swr_alloc_set_opts(nullptr,
        encoder.getChannelLayout(),
        encoder.getSampleFormat(),
        encoder.getSampleRate(),
        decoder.getChannelLayout(),
        decoder.getSampleFormat(),
        decoder.getSampleRate(),
        0,
        nullptr);
    if (!swrTemp)
    {
        throw std::runtime_error("Could not create SWR context");
    }

    std::unique_ptr<SwrContext, std::function<void(SwrContext*)>> swr(swrTemp, [](SwrContext* s) { swr_free(&s); });
    if (swr_init(swr.get()) != 0)
    {
        throw std::runtime_error("Could not initialize the SWR context");
    }

    const int bufferSamplesPerChannel = 44100;
    const int sampleSize = av_get_bytes_per_sample(encoder.getSampleFormat());
    const int channelCount = encoder.getChannelCount();
    std::unique_ptr<unsigned char[]> swrBuffer(new unsigned char[channelCount * sampleSize * bufferSamplesPerChannel]);
    std::vector<unsigned char*> outPtrs(channelCount, nullptr);
    if (av_samples_fill_arrays(outPtrs.data(), nullptr, swrBuffer.get(), channelCount, bufferSamplesPerChannel, encoder.getSampleFormat(), 0) < 0)
    {
        throw std::runtime_error("Could not set up the SWR buffer pointers");
    }

    int64_t totalSamplesRead = 0;
    AVFrame* frame;
    while (frame = decoder.decodeFrame())
    {
        int numSamplesOut = swr_convert(swr.get(), outPtrs.data(), bufferSamplesPerChannel, (const uint8_t**)frame->data, frame->nb_samples);
        if (numSamplesOut < 0)
        {
            throw std::runtime_error("Could not convert the audio data");
        }
        encoder.processSamples((const unsigned char**)outPtrs.data(), numSamplesOut);
        totalSamplesRead += numSamplesOut;

        if (callback)
        {
            float t = (float)((totalSamplesRead * AV_TIME_BASE) / decoder.getSampleRate()) / (float)decoder.getDuration();
            if (!callback(t))
            {
                return encoder.getEncodedData();
            }
        }
    }

    int numSamplesOut = 0;
    do {
        // This will conveniently flush the encoder too because we will get a case where there
        // are no samples in SWR and numSamplesOut will be zero
        numSamplesOut = swr_convert(swr.get(), outPtrs.data(), bufferSamplesPerChannel, nullptr, 0);
        if (numSamplesOut < 0)
        {
            throw std::runtime_error("Could not convert the audio data");
        }
        encoder.processSamples((const unsigned char**)outPtrs.data(), numSamplesOut);
        totalSamplesRead += numSamplesOut;

        if (callback)
        {
            float t = (float)((totalSamplesRead * AV_TIME_BASE) / decoder.getSampleRate()) / (float)decoder.getDuration();
            if (!callback(t))
            {
                return encoder.getEncodedData();
            }
        }
    } while (numSamplesOut > 0);

    if (callback)
    {
        callback(1);
    }

    return encoder.getEncodedData();
}
