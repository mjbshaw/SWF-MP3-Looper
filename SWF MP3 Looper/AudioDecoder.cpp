#include "AudioDecoder.hpp"

#include <new>
#include <stdexcept>

AudioDecoder::AudioDecoder(const std::string& source) : format(nullptr, [](AVFormatContext* f) { avformat_close_input(&f); }),
	codec(nullptr, avcodec_close),
	frame(nullptr, av_free),
	audioStream(nullptr)
{
	av_init_packet(&decodingPacket);
	decodingPacket.data = nullptr;
	decodingPacket.size = 0;

	av_init_packet(&readingPacket);
	readingPacket.data = nullptr;
	readingPacket.size = 0;

	frame.reset(avcodec_alloc_frame());
	if (!frame)
	{
		throw std::bad_alloc();
	}

	AVFormatContext* ctx = nullptr;
	if (avformat_open_input(&ctx, source.c_str(), nullptr, nullptr) != 0)
	{
		throw std::runtime_error("Error opening the file");
	}
	format.reset(ctx);

	if (avformat_find_stream_info(format.get(), nullptr) < 0)
	{
		throw std::runtime_error("Error finding the stream info");
	}

	AVCodec* cdc = nullptr;
	int stream = av_find_best_stream(format.get(), AVMEDIA_TYPE_AUDIO, -1, -1, &cdc, 0);
	if (stream < 0)
	{
		throw std::runtime_error("No good audio stream");
	}
	audioStream = format->streams[stream];

	audioStream->codec->codec = cdc;
	if (avcodec_open2(audioStream->codec, audioStream->codec->codec, nullptr) != 0)
	{
		throw std::runtime_error("Couldn't open the context with the decoder");
	}
	codec.reset(audioStream->codec);
}

uint64_t AudioDecoder::getChannelLayout() const
{
	if (codec->channel_layout <= 0)
	{
		return av_get_default_channel_layout(codec->channels);
	}
	
	return codec->channel_layout;
}

AVSampleFormat AudioDecoder::getSampleFormat() const
{
	return codec->sample_fmt;
}

int AudioDecoder::getSampleRate() const
{
	return codec->sample_rate;
}

int64_t AudioDecoder::getDuration() const
{
	return format->duration;
}

int AudioDecoder::getDelay() const
{
	return codec->delay;
}

AVFrame* AudioDecoder::decodeFrame()
{
	if (decodingPacket.size > 0)
	{
		int gotFrame = 0;
		int result = avcodec_decode_audio4(codec.get(), frame.get(), &gotFrame, &decodingPacket);

		if (result >= 0 && gotFrame)
		{
			decodingPacket.size -= result;
			decodingPacket.data += result;
			return frame.get();
		}
		else
		{
			decodingPacket.size = 0;
			decodingPacket.data = nullptr;
		}
	}

	av_free_packet(&readingPacket);

	while (av_read_frame(format.get(), &readingPacket) == 0)
	{
		if (readingPacket.stream_index == audioStream->index)
		{
			decodingPacket = readingPacket;

			int gotFrame = 0;
			int result = avcodec_decode_audio4(codec.get(), frame.get(), &gotFrame, &decodingPacket);

			if (result >= 0 && gotFrame)
			{
				decodingPacket.size -= result;
				decodingPacket.data += result;
				return frame.get();
			}
			else
			{
				decodingPacket.size = 0;
				decodingPacket.data = nullptr;
			}
		}

		av_free_packet(&readingPacket);
	}

	if (codec->codec->capabilities & CODEC_CAP_DELAY)
	{
		av_init_packet(&readingPacket);

		int gotFrame = 0;
		if (avcodec_decode_audio4(codec.get(), frame.get(), &gotFrame, &readingPacket) >= 0 && gotFrame)
		{
			return frame.get();
		}
	}

	return nullptr;
}