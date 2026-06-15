#include "file_io.h"
#include <fstream>
#include <spdlog/spdlog.h>

namespace viewer {

std::string read_file_text(const std::filesystem::path& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        spdlog::error("Failed to open file: {}", path.string());
        return {};
    }
    return {std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
}

std::vector<char> read_file_binary(const std::filesystem::path& path) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        spdlog::error("Failed to open file: {}", path.string());
        return {};
    }
    auto size = file.tellg();
    std::vector<char> buffer(size);
    file.seekg(0);
    file.read(buffer.data(), size);
    return buffer;
}

} // namespace viewer
