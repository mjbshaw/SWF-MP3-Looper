#include "Transcode.hpp"
#include "AudioDecoder.hpp"
#include "AudioEncoder.hpp"

extern "C"
{
#include <libswresample/swresample.h>
}

std::vector<unsigned char> transcode(const std::string& source, int sampleRate, int audioQuality, int vbrQuality, std::function<void(float)>)
{
	std::vector<unsigned char> buffer;

	std::unique_ptr<SwrContext, std::function<void(SwrContext*)>> swr(nullptr, [](SwrContext* s) { swr_free(&s); });

	AudioDecoder decoder(source);
	AudioEncoder encoder(sampleRate, audioQuality, vbrQuality);

	return buffer;
}
