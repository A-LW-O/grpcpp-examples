#include <fstream>
#include <sstream>
#include <stdexcept>

#include "helper.h"

namespace helper {

std::string ReadFile(const std::string& file_path) {
    std::ifstream file(file_path);
    if (!file.is_open()) {
        throw std::runtime_error("Unable to open file: " + file_path);
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

}
