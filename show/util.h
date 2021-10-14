#pragma once

#include <cmath>
#include <memory>
#include <span>
#include <string>
#include <string_view>

namespace util
{
auto count_digits(auto n) noexcept -> size_t
{
#pragma warning(suppress : 4244 26467)
	return std::floor(std::log10(n)) + 1;
}

auto make_ntfs_compliant(std::string& in) -> void;

class Args
{
	std::span<char*> cmd_line;

  public:
	[[nodiscard]] auto num_of_arguments_left() const noexcept -> size_t;
	auto			   pop_front() noexcept -> std::string_view;
	Args(int argc, char** argv) noexcept : cmd_line(argv + 1, argc - 1) {};
};

} // namespace util

namespace g
{
extern std::unique_ptr<util::Args> ARGUMENTS;
} // namespace g
