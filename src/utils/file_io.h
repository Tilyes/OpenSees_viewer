// =============================================================================
// 文件: utils/file_io.h
// 作用: file_io：文件读写工具。read_file_binary(path) 读二进制文件（shader SPIR-V），read_file_text(path) 读文本
// =============================================================================

#pragma once

#include <string>
#include <filesystem>

namespace viewer {

std::string read_file_text(const std::filesystem::path& path);
std::vector<char> read_file_binary(const std::filesystem::path& path);

} // namespace viewer
