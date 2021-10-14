#pragma once

#include <cassert>
#include <filesystem>
#include <map>
#include <string>

class Config
{
	using map = std::map<std::string, std::string>;

	Config() noexcept;
	~Config() = default;

	static auto get_config_dir() -> std::filesystem::path;

  public:
	Config(const Config&) = delete;
	Config(Config&&)	  = delete;
	auto operator=(const Config&) -> Config& = delete;
	auto operator=(Config&&) -> Config& = delete;

	map			m_settings;
	auto		create_new_key(std::string_view key, std::string_view value) -> bool;
	static auto get_instance() noexcept -> Config&;

	auto get_lang() -> std::string;
	auto get_api_key() -> std::string;
};
