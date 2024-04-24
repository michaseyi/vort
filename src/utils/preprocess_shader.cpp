#include <cassert>
#include <set>
#include <sstream>

#include "utils.hpp"

static std::string _preprocessShader(std::string content,
                                     std::set<std::string>& already_imported) {
  std::stringstream current_shader(content);
  std::stringstream ss;

  std::string line_buffer;
  while (!current_shader.eof()) {
    std::getline(current_shader, line_buffer);
    if (line_buffer.starts_with("#import ")) {
      auto start = line_buffer.find_first_of("\"");
      auto end = line_buffer.find_last_of("\"");

      assert(start != std::string::npos && end != std::string::npos);
      std::string path =
          line_buffer.substr(start + 1, end - start - 1) + ".wgsl";

      if (already_imported.find(path) == already_imported.end()) {
        already_imported.emplace(path);
        ss << _preprocessShader(load_shader_code(path), already_imported);
      }

    } else {
      ss << line_buffer << "\n";
    }
  }
  return ss.str();
}

std::string preprocess_shader(std::string content) {
  std::set<std::string> already_imported;

  return _preprocessShader(std::move(content), already_imported);
}
