#ifndef AUDIO_DECODER_HPP
#define AUDIO_DECODER_HPP

extern "C"
{
struct AVFrame;
struct AVFormatContext;
struct AVStream;
struct AVCodecContext;
}

#include <string>
#include <memory>
#include <functional>

class AudioDecoder
{
public:
	AudioDecoder(const std::string& source);

private:
	// Visual Studio 2010 doesn't allow deletion of these...
	AudioDecoder(const AudioDecoder&) {}
	AudioDecoder& operator=(const AudioDecoder&) {return *this;}

	AVStream* audioStream;
	// Order of these members is important (so that they are destroyed in the right order)
	std::unique_ptr<AVFrame, std::function<void(void*)>> frame;
	std::unique_ptr<AVCodecContext, std::function<int(AVCodecContext*)>> codec;
	std::unique_ptr<AVFormatContext, std::function<void(AVFormatContext*)>> format;
};

#endif