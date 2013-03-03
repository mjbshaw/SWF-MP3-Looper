#ifndef AUDIO_DECODER_HPP
#define AUDIO_DECODER_HPP

struct AVFrame;
struct AVFormatContext;
struct AVStream;
struct AVCodecContext;
struct SwrContext;

#include <memory>

class AudioDecoder
{
public:
	AudioDecoder();

private:
	AudioDecoder(const AudioDecoder&) {}
	AudioDecoder& operator=(const AudioDecoder&) {return *this;}

	// Order of these members is important (so that they are destroyed in the right order)
	std::unique_ptr<AVFrame> frame;
};

#endif