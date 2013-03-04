#ifndef AUDIO_DECODER_HPP
#define AUDIO_DECODER_HPP

extern "C"
{
struct AVFrame;
struct AVFormatContext;
struct AVStream;
struct AVCodecContext;
struct SwrContext;
}

#include <memory>
#include <functional>

class AudioDecoder
{
public:
	AudioDecoder();

private:
	AudioDecoder(const AudioDecoder&) {}
	AudioDecoder& operator=(const AudioDecoder&) {return *this;}

	AVStream* audioStream;
	// Order of these members is important (so that they are destroyed in the right order)
	std::unique_ptr<AVFrame, std::function<void(void*)>> frame;
	std::unique_ptr<AVCodecContext, std::function<int(AVCodecContext*)>> codec;
	std::unique_ptr<AVFormatContext, std::function<void(AVFormatContext*)>> format;
	std::unique_ptr<SwrContext, std::function<void(SwrContext*)>> swr;

};

#endif