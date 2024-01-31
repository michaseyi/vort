#include <cassert>
#include <sstream>

#include "utils.hpp"

std::string preprocessShader(std::string &content) {
    std::stringstream current_shader(content);
    std::stringstream ss;

    std::string line_buffer;
    while (!current_shader.eof()) {
        std::getline(current_shader, line_buffer);
        if (line_buffer.starts_with("#import")) {
            auto start = line_buffer.find_first_of("\"");
            auto end = line_buffer.find_last_of("\"");

            assert(start != std::string::npos && end != std::string::npos);
            std::string path = line_buffer.substr(start + 1, end - start - 1) + ".wgsl";

            std::string rawSrc = loadString(path);
            ss << preprocessShader(rawSrc);

        } else {
            ss << line_buffer << "\n";
        }
    }
    return ss.str();
}