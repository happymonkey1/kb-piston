#pragma once

#include "kb/piston/core/types.h"

#include <fstream>
#include <string>
#include <filesystem>

namespace kb::piston::util
{ // start namespace kb::piston::util

inline auto read_file_into_buffer(const std::filesystem::path& p_file_path) -> std::string
{
    std::ifstream file{ p_file_path, std::ios::in };

    std::string buffer{};
    if (file)
    {
        file.seekg(0, std::ios::end);
        buffer.resize(file.tellg());
        file.seekg(0, std::ios::beg);
        file.read(buffer.data(), static_cast<i64>(buffer.size()));
    }

    return buffer;
}

} // end namespace kb::piston::util