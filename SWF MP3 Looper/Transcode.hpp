#ifndef TRANSCODE_HPP
#define TRANSCODE_HPP

#include <functional>
#include <string>
#include <vector>

std::vector<unsigned char> transcode(const std::string& source, int sampleRate, int audioQuality, int vbrQuality, std::function<void(float)> = std::function<void(float)>());

#endif