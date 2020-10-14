#pragma once

namespace os
{
	auto read_binary_file(const std::filesystem::path &file_path) -> std::vector<uint8_t>;
}
