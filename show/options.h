#pragma once

#include <cassert>
#include <filesystem>
#include <map>
#include <string>

class Config
{
	using map = std::map<std::string, std::string>;

	Config();
	~Config() = default;

	static auto get_config_dir() -> std::filesystem::path;

  public:
	Config(const Config&)					 = delete;
	Config(Config&&)						 = delete;
	auto operator=(const Config&) -> Config& = delete;
	auto operator=(Config&&) -> Config&		 = delete;

	map m_settings;
	struct Key_value
	{
		std::string_view key;
		std::string_view value;
	};
	auto		create_new_key(Key_value kv_pair) -> bool;
	static auto get_instance() -> Config&;

	[[nodiscard]] auto get_lang() -> std::string;
	[[nodiscard]] auto get_api_key() -> std::string;
	[[nodiscard]] auto get_adult() -> std::string;
};
