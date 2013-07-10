#include "SwfMp3Sound.hpp"

#include <vector>
#include <fstream>
#include <stdexcept>

/*
This code is far from ideal. I was working on a SWF compiler/decompiler called libswfer,
but it's far from complete and I don't currently have time to work on it. libswfer is
able to generate all the bytes for a gaplessly looped MP3 in a SWF, but I don't want to
include libswfer in this project since libswfer is so far from being even half complete.
Instead, I've manually written the bytes here. Once libswfer matures in the potentially
indefinite future, I'll replace all of this messy bit twiddling code.
*/

void SwfMp3Sound::saveSwf(const std::string& path) const
{
	// Assumes little endian
	// Also assumes char == 8 bits, short == 16 bits, int == 32 bits
	std::ofstream file(path, std::ios::binary);

	if (!file)
	{
		throw std::runtime_error("Could not open SWF file for writing");
	}

	char header[] = {
		'F', 'W', 'S', // Non compressed flash file
		10,            // SWF version
		0, 0, 0, 0,    // File length (fill it in later)
		8, 0,          // Frame size (0, 0, 0, 0) (we have no graphics...)
		0, 24,         // FPS (doesn't matter for us; we'll say 24.0)
		0, 0           // Frame count (we have no frames)
	};
	if (!file.write(header, sizeof(header)))
	{
		throw std::runtime_error("Error when writing the SWF's header");
	}
	
	char sr = sampleRate == 5512 ? 0 : // Note: sample rate 5512 is illegale for MP3
		sampleRate == 11025 ? 1 :
		sampleRate == 22050 ? 2 :
		sampleRate == 44100 ? 3 : -1;
	if (sr < 0)
	{
		throw std::invalid_argument("Sample rate is not 5512, 11025, 22050, or 44100");
	}

	char ss = sampleSize == 8 ? 0 :
		sampleSize == 16 ? 1 : -1;
	if (ss < 0)
	{
		throw std::invalid_argument("Sample size is not 8 or 16");
	}

	char cl = channelCount == 1 ? 0 :
		channelCount == 2 ? 1 : -1;
	if (cl < 0)
	{
		throw std::invalid_argument("Channel count is not 1 (mono) or 2 (stereo)");
	}

	// DefineSound tag
	char defineSound[] = {
		1, 0,     // ID/tag
		(mp3? 2 : 3)  << 4 | // MP3 or PCM Little endian
		sr << 2 | // Sample rate
		ss << 1 | // Sample size
		cl        // Channel layout
	};
	short defineSoundHeader = 14 << 6 | 0x3f;
	int defineSoundSize = sizeof(defineSound) + 4 + data.size() + (mp3 ? 2 : 0);
	if (!file.write((const char*)&defineSoundHeader, 2) ||
		!file.write((const char*)&defineSoundSize, 4) ||
		!file.write(defineSound, sizeof(defineSound)) ||
		!file.write((const char*)&sampleCount, 4) ||
		(mp3 && !file.write((const char*)&seekSamples, 2)) || // This is the key to gapless looping
		!file.write((const char*)data.data(), data.size()))
	{
		throw std::runtime_error("Error when writing the SWF's sound data");
	}

	if (className.empty())
	{
		throw std::invalid_argument("Class name is empty, but it must not be");
	}

	// SymbolClass tag
	char symbolClass[] =  {
		1, 0, // Symbol count (1 symbol)
		1, 0 // Symbol ID/tag
	};
	short symbolClassHeader = 76 << 6 | 0x3f;
	int symbolClassSize = sizeof(symbolClass) + className.size() + 1;
	if (!file.write((const char*)&symbolClassHeader, 2) ||
		!file.write((const char*)&symbolClassSize, 4) ||
		!file.write(symbolClass, sizeof(symbolClass)) ||
		!file.write(className.c_str(), className.size() + 1))
	{
		throw std::runtime_error("Error when writing the SWF's symbol table");
	}

	// End tag
	char end[] = {0, 0};
	if (!file.write(end, sizeof(end)))
	{
		throw std::runtime_error("Error when writing the SWF's end tag");
	}

	// Write the file size back into the header
	int length = file.tellp();
	if (length < 0 ||
		!file.seekp(4) ||
		!file.write((const char*)&length, 4))
	{
		throw std::runtime_error("Error writing the SWF's size");
	}
}