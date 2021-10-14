#include "options.h"
#include "tree_util.h"

#include <algorithm>
#include <array>
#include <cassert>
#include <filesystem>
#include <format>
#include <fstream>
#include <ranges>
#include <cstdlib>
#include <vector>

void tree::Set::func()
{
	auto const key {Param::string()};
	auto const value {Param::string()};
	Config::get_instance().create_new_key(key, value);
}

void tree::Get::func()
{
	std::string const key {Param::string()};

	auto& config = Config::get_instance();

	if(config.m_settings.contains(std::string {key}))
	{
		std::format_to(std::ostream_iterator<char>(std::cout), "key:{}\nvalue:{}", key, config.m_settings[key]);
	}
	else
	{
		std::format_to(std::ostream_iterator<char>(std::cout), "key:{}\nvalue:{}", key, "undefined");
	}
}

Config::Config() noexcept
{
	namespace fs = std::filesystem;
	using P		 = fs::path;
	size_t const max_size {1024};

	try
	{
		auto const is_small_file = [](fs::directory_entry const& p)
		{ return p.exists() && p.is_regular_file() && (p.file_size() <= max_size); };
		P const		   config_dir = Config::get_config_dir();
		std::vector<P> files {};
		std::ranges::copy_if(fs::directory_iterator(config_dir), std::back_inserter(files), is_small_file);

		std::map<std::string, std::string> settings;
		auto const						   load_file = [&settings](P const& p)
		{
			if(std::ifstream file {p}; file.is_open()) { file >> settings[p.filename().string()]; }
		};

		std::ranges::for_each(files, load_file);
		m_settings = settings;
	}
	catch(...)
	{
		//std::cout << "Couldn't load configuration files";
	}
}

auto Config::get_config_dir() -> std::filesystem::path

{
	namespace fs = std::filesystem;
	using P		 = fs::path;

	size_t const buffer_size {512};

	std::array<char, buffer_size> b {};
	size_t						  writen_char_count {};
	errno_t const				  err {getenv_s(&writen_char_count, b.data(), b.size(), "USERPROFILE")};
	assert(err == 0);

	P		   home_dir {b.data()};
	P		   config_dir {home_dir / ".show"};
	auto const config_dir_exists = fs::is_directory(config_dir);
	if(!config_dir_exists)
	{
		std::error_code ec {};
		fs::create_directory(config_dir, ec);
		if(ec)
		{
			std::cout << ec.message();
			std::exit(EXIT_FAILURE);
		}
	}

	return config_dir;
}

auto Config::get_instance() noexcept -> Config&
{
	static Config cfg {};
	return cfg;
}

auto Config::get_lang() -> std::string
{
	std::string const key {"lang"};
	return m_settings.contains(key) ? std::string {"language=" + m_settings[key]} : "language=en-US";
}

auto Config::get_api_key() -> std::string
{
	std::string const key {"api_key"};
	return m_settings.contains(key) ? std::string {"api_key=" + m_settings[key]} : "api_key=xxx";
}

auto Config::create_new_key(std::string_view key, std::string_view value) -> bool
{
	namespace fs	   = std::filesystem;
	using P			   = fs::path;
	P const config_dir = this->get_config_dir();

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
