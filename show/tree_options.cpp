#include "options.h"
#include "tree_util.h"

#include <algorithm>
#include <array>
#include <cassert>
#include <cstdlib>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <ranges>
#include <vector>

void tree::Set::func()
{
	auto [key, value] = Param::get_arguments<arguments>();
	Config::get_instance().create_new_key(Config::Key_value {.key = key, .value = value});
}

void tree::Get::func()
{
	auto [key] {Param::get_arguments<arguments>()};
	auto& config {Config::get_instance()};

	if(config.m_settings.contains(std::string {key}))
	{
		std::format_to(std::ostream_iterator<char> {std::cout}, "key:{}\nvalue:{}", key, config.m_settings[std::string {key}]);
	}
	else
	{
		std::format_to(std::ostream_iterator<char>(std::cout), "key:{}\nvalue:{}", key, "undefined");
	}
}

Config::Config()
{
	using namespace std::filesystem;
	size_t constexpr max_size {1024};
	try
	{
		auto const		  is_small_file {[](directory_entry const& dir_ent)
									 { return dir_ent.exists() && dir_ent.is_regular_file() && (dir_ent.file_size() <= max_size); }};
		path const		  config_dir {Config::get_config_dir()};
		std::vector<path> files {};
		std::ranges::copy_if(directory_iterator(config_dir), std::back_inserter(files), is_small_file);

		std::map<std::string, std::string> settings;

		auto const load_file = [&settings](path const& file_path)
		{
			if(std::ifstream file {file_path}; file.is_open())
			{
				file >> settings[file_path.filename().string()];
			}
		};

		std::ranges::for_each(files, load_file);
		m_settings = settings;
	}
	catch(std::exception const& e)
	{
		std::format_to(std::ostream_iterator<char> {std::cout}, "The configuration file can't be loaded due to: {}\n", e.what());
	}
}

auto Config::get_config_dir() -> std::filesystem::path
{
	namespace fs = std::filesystem;

	size_t constexpr buffer_size {512};
	std::array<char, buffer_size> buffer {};
	size_t						  writen_char_count {};
	errno_t const				  error {getenv_s(&writen_char_count, buffer.data(), buffer.size(), "USERPROFILE")};
	if(error != 0)
	{
		throw;
	}

	fs::path   home_dir {buffer.data()};
	fs::path   config_dir {home_dir / ".show"};
	auto const config_dir_exists = fs::is_directory(config_dir);
	if(!config_dir_exists)
	{
		std::error_code error_code {};
		fs::create_directory(config_dir, error_code);
		if(error_code)
		{
			std::cout << error_code.message();
			std::exit(EXIT_FAILURE);
		}
	}

	return config_dir;
}

auto Config::get_instance() -> Config&
{
	static Config cfg {};
	return cfg;
}

auto Config::get_lang() -> std::string
{
	std::string const key {"lang"};
	return m_settings.contains(key) ? m_settings[key] : "language=en-US";
}

auto Config::get_api_key() -> std::string
{
	std::string const key {"api_key"};
	return m_settings.contains(key) ? m_settings[key] : "api_key=xxx";
}

auto Config::get_adult() -> std::string
{
	std::string const key {"adult"};
	return m_settings.contains(key) ? std::string {"include_adult=" + m_settings[key]} : "include_adult=false";
}

auto Config::create_new_key(Key_value kv_pair) -> bool
{
	auto const [key, value] = kv_pair;
	namespace fs			= std::filesystem;
	fs::path const config_dir {this->get_config_dir()};
	try
	{
		std::ofstream file {config_dir / key};
		file << value;
	}
	catch(...)
	{
		return false;
	}

	return true;
}
