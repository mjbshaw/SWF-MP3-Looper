#ifndef AUDIO_ENCODER_HPP
#define AUDIO_ENCODER_HPP

extern "C"
{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/audio_fifo.h>
}

#include <memory>
#include <functional>
#include <vector>

class AudioEncoder
{
public:
	AudioEncoder(int sampleRate, int audioQuality, int vbrQuality);
	
	int getChannelCount() const;
	uint64_t getChannelLayout() const;
	AVSampleFormat getSampleFormat() const;
	int getSampleRate() const;

	void processSamples(const unsigned char** buffer, int sampleCount);

	const std::vector<unsigned char>& getEncodedData() const;

private:
	AudioEncoder(const AudioEncoder&) {}
	AudioEncoder& operator= (const AudioEncoder&) {return *this;}

	std::vector<unsigned char> encodedData;
	AVCodec* codec;

	// Order of these members is important (so that they are destroyed in the right order)
	std::unique_ptr<AVFrame, std::function<void(void*)>> frame;
	std::unique_ptr<unsigned char[]> frameBuffer;
	std::unique_ptr<AVAudioFifo, std::function<void(AVAudioFifo*)>> fifo;
	std::unique_ptr<AVCodecContext, std::function<void(AVCodecContext*)>> context;
};

#endif