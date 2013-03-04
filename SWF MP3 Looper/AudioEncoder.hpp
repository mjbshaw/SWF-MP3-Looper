#ifndef AUDIO_ENCODER_HPP
#define AUDIO_ENCODER_HPP

extern "C"
{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
}

#include <memory>
#include <functional>

class AudioEncoder
{
public:
	AudioEncoder(int sampleRate, int audioQuality, int vbrQuality);
	
	uint64_t getChannelLayout() const;
	AVSampleFormat getSampleFormat() const;
	int getSampleRate() const;

private:
	AudioEncoder(const AudioEncoder&) {}
	AudioEncoder& operator= (const AudioEncoder&) {return *this;}

	AVCodec* codec;

	// Order of these members is important (so that they are destroyed in the right order)
	std::unique_ptr<AVFrame, std::function<void(AVFrame*)>> frame;
	std::unique_ptr<unsigned char[]> frameBuffer;
	std::unique_ptr<AVCodecContext, std::function<void(AVCodecContext*)>> context;
};

#endif