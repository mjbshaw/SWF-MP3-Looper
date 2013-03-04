#include "AudioDecoder.hpp"

extern "C"
{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswresample/swresample.h>
}

#include <new>
#include <stdexcept>

AudioDecoder::AudioDecoder() : audioStream(nullptr),
	frame(nullptr, av_free),
	codec(nullptr, avcodec_close),
	format(nullptr, [](AVFormatContext* f) { avformat_close_input(&f); }),
	swr(nullptr, [](SwrContext* s) { swr_free(&s); })
{
	frame.reset(avcodec_alloc_frame());
	if (!frame)
	{
		throw std::bad_alloc();
	}

	AVFormatContext* ctx = NULL;
	if (avformat_open_input(&ctx, "path", nullptr, nullptr) != 0)
	{
		throw std::runtime_error("Error opening the file");
	}
	format.reset(ctx);

	if (avformat_find_stream_info(format.get(), NULL) < 0)
	{
		throw std::runtime_error("Error finding the stream info");
	}

	AVCodec* cdc = NULL;
	int stream = av_find_best_stream(format.get(), AVMEDIA_TYPE_AUDIO, -1, -1, &cdc, 0);
	if (stream < 0)
	{
		throw std::runtime_error("No good audio stream");
	}
	audioStream = format->streams[stream];

	audioStream->codec->codec = cdc;
	if (avcodec_open2(audioStream->codec, audioStream->codec->codec, NULL) != 0)
	{
		throw std::runtime_error("Couldn't open the context with the decoder");
	}
	codec.reset(audioStream->codec);
}