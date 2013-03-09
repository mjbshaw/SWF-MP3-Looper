#ifndef AUDIO_DECODER_HPP
#define AUDIO_DECODER_HPP

extern "C"
{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
}

#include <string>
#include <memory>
#include <functional>

class AudioDecoder
{
public:
	AudioDecoder(const std::string& source);

	uint64_t getChannelLayout() const;
	AVSampleFormat getSampleFormat() const;
	int getSampleRate() const;
	int64_t getDuration() const;

	AVFrame* decodeFrame();

private:
	// Visual Studio 2010 doesn't allow deletion of these...
	AudioDecoder(const AudioDecoder&) {}
	AudioDecoder& operator=(const AudioDecoder&) {return *this;}

	// Order of these members is important (so that they are destroyed in the right order)
	std::unique_ptr<AVFormatContext, std::function<void(AVFormatContext*)>> format;
	std::unique_ptr<AVCodecContext, std::function<int(AVCodecContext*)>> codec;
	std::unique_ptr<AVFrame, std::function<void(void*)>> frame;

	AVStream* audioStream;

	AVPacket decodingPacket;
	AVPacket readingPacket;
};

#endif