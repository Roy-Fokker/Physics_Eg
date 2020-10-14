#include "helper.h"

using namespace os;

auto os::read_binary_file(const std::filesystem::path &file_path) -> std::vector<uint8_t>
{
	auto buffer = std::vector<uint8_t>{};

	auto file = std::ifstream(file_path, std::ios::in | std::ios::binary);
	assert(file.is_open());
	file.unsetf(std::ios::skipws);

	file.seekg(0, std::ios::end);
	buffer.reserve(file.tellg());
	file.seekg(0, std::ios::beg);

	std::copy(std::istream_iterator<uint8_t>(file),
	          std::istream_iterator<uint8_t>(),
	          std::back_inserter(buffer));

	buffer.shrink_to_fit();

	return buffer;
}