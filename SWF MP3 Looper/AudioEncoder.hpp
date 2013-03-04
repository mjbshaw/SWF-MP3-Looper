#ifndef AUDIO_ENCODER_HPP
#define AUDIO_ENCODER_HPP

extern "C"
{
struct AVFrame;
struct AVCodec;
struct AVCodecContext;
}

class AudioEncoder
{
public:
	AudioEncoder();

private:
	AudioEncoder(const AudioEncoder&) {}
	AudioEncoder& operator= (const AudioEncoder&) {return *this;}
};

#endif