#include "Transcode.hpp"
#include "AudioDecoder.hpp"
#include "AudioEncoder.hpp"

extern "C"
{
#include <libswresample/swresample.h>
}

#include <stdexcept>

std::vector<unsigned char> transcode(const std::string& source, int sampleRate, int audioQuality, int vbrQuality, std::function<void(float)>)
{
	std::vector<unsigned char> buffer;

	AudioDecoder decoder(source);
	AudioEncoder encoder(sampleRate, audioQuality, vbrQuality);

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

	AVFrame* frame;
	while (frame = decoder.decodeFrame())
	{
	}
	
	return buffer;
}
