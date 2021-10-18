#pragma once

#include <memory>
#include <span>
#include <string>
#include <string_view>

namespace util
{
auto count_digits(__int64 n) noexcept -> size_t;


auto make_ntfs_compliant(std::string& in) -> void;

struct Args
{
	std::span<char*> cmd_line;

	[[nodiscard]] auto num_of_arguments_left() const noexcept -> size_t;
	auto			   pop_front() noexcept -> std::string_view;
	Args(int argc, char** argv) noexcept : cmd_line(argv, argc) 
	{
	   cmd_line = cmd_line.subspan(1);	
	};
};

} // namespace util

namespace g
{
extern std::unique_ptr<util::Args> ARGUMENTS;
} // namespace g
