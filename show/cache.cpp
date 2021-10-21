#include "cache.h"

#include <cassert>
#include <fstream>
#include <sstream>

namespace cache
{
auto add(std::string_view json, std::string_view url) -> void
{
	auto const hash {std::hash<std::string_view> {}(url)};
	auto	   cache_dir {cache::get_cache_folder()};

	std::stringstream ss {};
	ss << std::hex << hash;

	cache_dir = cache_dir / ss.str();

	if(std::ofstream file {cache_dir, std::ofstream::trunc | std::ofstream::binary}; file.is_open())
	{
		file << json;
	}
}

auto get(std::string_view url) -> std::string
{
	namespace fs = std::filesystem;
	auto const hash {std::hash<std::string_view> {}(url)};
	auto	   cache_dir {cache::get_cache_folder()};

	std::stringstream ss {};
	ss << std::hex << hash;

	auto const file_dir {cache_dir / ss.str()};

	assert(fs::is_regular_file(file_dir));

	std::string json {};
	if(std::ifstream file {file_dir, std::ofstream::binary}; file.is_open())
	{
		json = std::string {std::istreambuf_iterator<char> {file}, {}};
	}
	return json;
}

auto is_stored(std::string_view url) -> bool
{
	namespace fs = std::filesystem;
	auto const hash {std::hash<std::string_view> {}(url)};
	auto	   cache_dir {cache::get_cache_folder()};

	std::stringstream ss {};
	ss << std::hex << hash;

	auto const file {cache_dir / ss.str()};
	auto const is_file {fs::is_regular_file(file)};

	return is_file;
}

auto get_cache_folder() -> std::filesystem::path
{
	namespace fs = std::filesystem;
	fs::path cache {fs::temp_directory_path() / "show"};
	if(!fs::is_directory(cache))
	{
		fs::create_directory(cache);
		assert(fs::is_directory(cache));
	}

	return cache;
}
} // namespace cache
