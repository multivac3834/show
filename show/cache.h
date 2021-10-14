#pragma once
#include <filesystem>
#include <string_view>

namespace cache
{
auto			   add(std::string_view json, std::string_view url) -> void;
[[nodiscard]] auto is_stored(std::string_view url) -> bool;
[[nodiscard]] auto get(std::string_view url) -> std::string;
[[nodiscard]] auto get_cache_folder() -> std::filesystem::path;
}; // namespace Cache
