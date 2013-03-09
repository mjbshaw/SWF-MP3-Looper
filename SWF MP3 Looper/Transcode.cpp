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

	const int swrBufferSizeInSamples = 44100;
	const int sampleSize = av_get_bytes_per_sample(encoder.getSampleFormat());
	std::unique_ptr<unsigned char[]> swrBuffer(new unsigned char[sampleSize * swrBufferSizeInSamples]);
	unsigned char* outPtrs[SWR_CH_MAX] = {nullptr};
	outPtrs[0] = swrBuffer.get();

	AVFrame* frame;
	while (frame = decoder.decodeFrame())
	{
		int numSamplesOut = swr_convert(swr.get(), outPtrs, swrBufferSizeInSamples, (const uint8_t**)frame->data, frame->nb_samples);
		// Now pass off swrBuffer.data() to encoder
	}

	int numSamplesOut = 0;
	do {
		numSamplesOut = swr_convert(swr.get(), outPtrs, swrBufferSizeInSamples, (const uint8_t**)frame->data, frame->nb_samples);
		// Now pass off swrBuffer.data() to encoder
	} while (numSamplesOut > 0);
	
	return buffer;
}
