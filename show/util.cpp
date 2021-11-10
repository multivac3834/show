#include "util.h"
#include <cassert>

namespace g
{
std::unique_ptr<util::Args> ARGUMENTS {};
}

auto util::Args::num_of_arguments_left() const noexcept -> size_t
{
	return cmd_line.size();
}

auto util::Args::pop_front() noexcept -> std::string_view
{
	assert(cmd_line.size());
	std::string_view front = cmd_line.front();
	cmd_line			   = cmd_line.subspan(1);
	return front;
}

util::Args::Args(int argc, char** argv) noexcept : cmd_line(argv, argc)
{
	cmd_line = cmd_line.subspan(1);
}

constexpr auto util::count_digits(__int64 n) noexcept -> size_t
{
	size_t constexpr dec {10};
	size_t digits {};
	while(n != 0)
	{
		n /= dec;
		++digits;
	}
	return digits;
}

auto util::make_ntfs_compliant(std::string& in) -> void
{
	auto const bad_char = [](auto const& i)
	{ return i == '/' || i == '\\' || i == ':' || i == '*' || i == '?' || i == '"' || i == '<' || i == '>' || i == '|'; };
	in.erase(std::remove_if(in.begin(), in.end(), bad_char), in.end());
}

auto util::year_from_date(std::string const& date) -> std::string
{
	if(date.length() >= 4)
	{
		return std::string {date, 0, 4};
	}
	return date;
}