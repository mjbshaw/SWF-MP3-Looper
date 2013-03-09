#include "AudioEncoder.hpp"

#include <new>
#include <stdexcept>
#include <iostream>

AudioEncoder::AudioEncoder(int sampleRate, int audioQuality, int vbrQuality) : context(nullptr, [](AVCodecContext* c) { avcodec_close(c); av_free(c); }),
	fifo(nullptr, av_audio_fifo_free),
	frame(nullptr, av_free),
	codec(nullptr)
{
	const AVSampleFormat sampleFormat = AV_SAMPLE_FMT_S16;
	const int channelLayout = AV_CH_LAYOUT_STEREO;

	frame.reset(avcodec_alloc_frame());
	if (!frame)
	{
		throw std::bad_alloc();
	}

	codec = avcodec_find_encoder(AV_CODEC_ID_MP3);
	if (!codec)
	{
		throw std::runtime_error("Could not find an MP3 encoder");
	}

	if (codec->sample_fmts != nullptr)
	{
		bool found = false;
		for (int i = 0; codec->sample_fmts[i] != -1; ++i)
		{
			if (codec->sample_fmts[i] == sampleFormat)
			{
				found = true;
				break;
			}
		}

		if (!found)
		{
			throw std::runtime_error("Sample format is not supported");
		}
	}

	if (codec->channel_layouts != nullptr)
	{
		bool found = false;
		for (int i = 0; codec->channel_layouts[i] != 0; ++i)
		{
			if (codec->channel_layouts[i] == channelLayout)
			{
				found = true;
				break;
			}
		}

		if (!found)
		{
			throw std::runtime_error("Channel layout is not supported");
		}
	}

	if (codec->supported_samplerates != nullptr)
	{
		bool found = false;
		for (int i = 0; codec->supported_samplerates[i] != 0; ++i)
		{
			if (codec->supported_samplerates[i] == sampleRate)
			{
				found = true;
				break;
			}
		}

		if (!found)
		{
			throw std::runtime_error("Requested sample rate is not supported");
		}
	}

	context.reset(avcodec_alloc_context3(codec));
	if (!context)
	{
		//avcodec_close and av_free
		throw std::bad_alloc();
	}

	context->flags |= CODEC_FLAG_QSCALE; // VBR
	context->global_quality = vbrQuality * FF_QP2LAMBDA; // [0, 9] for lame encoder
	context->compression_level = audioQuality; // [0, 9] for lame encoder

	context->sample_rate = sampleRate;
	context->sample_fmt = sampleFormat;
	context->channel_layout = channelLayout;
	context->channels = av_get_channel_layout_nb_channels(context->channel_layout);

	if (avcodec_open2(context.get(), codec, nullptr) < 0)
	{
		throw std::runtime_error("Could not open codec");
	}

	fifo.reset(av_audio_fifo_alloc(context->sample_fmt, context->channels, context->frame_size));
	if (!fifo)
	{
		throw std::bad_alloc();
	}

	frame->nb_samples = context->frame_size;
	frame->format = context->sample_fmt;
	frame->channel_layout = context->channel_layout;
	frame->channels = context->channels;

	int bufferSize = av_samples_get_buffer_size(nullptr, frame->channels, frame->nb_samples, context->sample_fmt, 0);
	frameBuffer.reset(new unsigned char[bufferSize]);

	if (avcodec_fill_audio_frame(frame.get(), context->channels, context->sample_fmt, frameBuffer.get(), bufferSize, 0) < 0)
	{
		throw std::runtime_error("Could not set up the audio frame");
	}
}

int AudioEncoder::getChannelCount() const
{
	return context->channels;
}

uint64_t AudioEncoder::getChannelLayout() const
{
	return context->channel_layout;
}

AVSampleFormat AudioEncoder::getSampleFormat() const
{
	return context->sample_fmt;
}

int AudioEncoder::getSampleRate() const
{
	return context->sample_rate;
}

const std::vector<unsigned char>& AudioEncoder::getEncodedData() const
{
	return encodedData;
}

// Call with zero samples to flush the buffer
void AudioEncoder::processSamples(const unsigned char** buffer, int sampleCount)
{
	const int unitSize = getChannelCount() * av_get_bytes_per_sample(getSampleFormat());
	int totalSamples = 0;

	if (sampleCount > 0)
	{
		if (av_audio_fifo_write(fifo.get(), (void**)buffer, sampleCount) < 0)
		{
			std::cerr << "Error writing to the audio FIFO\n";
		}

		while (av_audio_fifo_size(fifo.get()) >= context->frame_size)
		{
			if (av_audio_fifo_read(fifo.get(), (void**)frame->extended_data, context->frame_size) > 0)
			{
				AVPacket pkt;
				av_init_packet(&pkt);
				pkt.data = nullptr;
				pkt.size = 0;

				int gotPacket = 0;
				if (avcodec_encode_audio2(context.get(), &pkt, frame.get(), &gotPacket) < 0)
				{
					std::cerr << "Error encoding audio frame\n";
				}
				else if (gotPacket)
				{
					encodedData.insert(encodedData.end(), pkt.data, pkt.data + pkt.size);
					av_free_packet(&pkt);
				}
			}
		}
	}

	if (sampleCount == 0)
	{
		// flush
		AVPacket pkt;
		av_init_packet(&pkt);
		pkt.data = nullptr;
		pkt.size = 0;
		int gotPacket = 0;

		while (av_audio_fifo_size(fifo.get()) > 0)
		{
			int count = av_audio_fifo_read(fifo.get(), (void**)frame->extended_data, context->frame_size);
			if (count > 0)
			{
				frame->nb_samples = count;

				if (avcodec_encode_audio2(context.get(), &pkt, frame.get(), &gotPacket) < 0)
				{
					std::cerr << "Error encoding audio frame\n";
				}
				else if (gotPacket)
				{
					encodedData.insert(encodedData.end(), pkt.data, pkt.data + pkt.size);
					av_free_packet(&pkt);
				}
			}
		}

		if (codec->capabilities & CODEC_CAP_DELAY)
		{
			while (avcodec_encode_audio2(context.get(), &pkt, nullptr, &gotPacket) == 0 && gotPacket)
			{
				encodedData.insert(encodedData.end(), pkt.data, pkt.data + pkt.size);
				av_free_packet(&pkt);
			}
		}
	}
}