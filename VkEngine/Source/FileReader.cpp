#include "pch.h"
#include "FileReader.h"

std::vector<char> FileReader::Read(const std::string& filename)
{
	std::ifstream file(filename, std::ios::ate | std::ios::binary);
	assert(file.is_open());

	const size_t fileSize = static_cast<size_t>(file.tellg());
	std::vector<char> buffer(fileSize);

	file.seekg(0);
	file.read(buffer.data(), fileSize);

	file.close();
	return buffer;
}
