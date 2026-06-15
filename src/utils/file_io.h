#pragma once

#include <string>
#include <filesystem>

namespace viewer {

std::string read_file_text(const std::filesystem::path& path);
std::vector<char> read_file_binary(const std::filesystem::path& path);

} // namespace viewer
