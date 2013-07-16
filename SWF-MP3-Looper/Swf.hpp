#ifndef SWF_HPP
#define SWF_HPP

#include <fstream>
#include <functional>
#include <string>
#include <vector>

class AudioDecoder;
class AudioEncoder;

class Swf
{
public:
    Swf(const Swf&) = delete;
    Swf& operator=(const Swf&) = delete;

    Swf();
    Swf(const std::string& filePath);
    ~Swf();

    void open(const std::string& filePath);
    void addSound(AudioDecoder& decoder, AudioEncoder& encoder, const std::string& className, std::function<bool(float)> callback = std::function<bool(float)>());
    void close();

private:
    std::ofstream file;
    std::vector<std::string> classNames;
};

#endif
