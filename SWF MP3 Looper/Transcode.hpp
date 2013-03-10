#ifndef TRANSCODE_HPP
#define TRANSCODE_HPP

#include <functional>
#include <vector>

class AudioDecoder;
class AudioEncoder;

std::vector<unsigned char> transcode(AudioDecoder& decoder, AudioEncoder& encoder, std::function<void(float)> callback = std::function<void(float)>());

#endif