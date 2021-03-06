#ifndef TRANSCODE_HPP
#define TRANSCODE_HPP

#include <cstdint>
#include <functional>
#include <vector>

class AudioDecoder;
class AudioEncoder;

const std::vector<std::uint8_t>& transcode(AudioDecoder& decoder, AudioEncoder& encoder, std::function<bool(float)> callback = std::function<bool(float)>());

#endif
