#pragma once

#include <cassert>

#include <bit>
#include <ranges>
#include <string>

#include <gsl/gsl-lite.hpp>

#pragma warning( disable: 26490 ) 

namespace utf
{
inline auto to_string(std::u8string const& utf_str) -> std::string
{
	return std::string(reinterpret_cast<char const*>(utf_str.data()), utf_str.size());
}

// Returns the number of codepoints
// utf8 character can consume more than a single CP
[[nodiscard]] auto count_codepoints(std::ranges::range auto string) noexcept -> size_t
{
	size_t ch_count {};
	for(unsigned char const ch : string)
	{
		auto const l_one = std::countl_one(ch);
		switch(l_one)
		{
		case 0:
		case 2:
		case 3:
		case 4:
			++ch_count;
			break;
		case 1: // case for trailing bytes in multi byte characters
		default:
			break;
		}
	}
	return ch_count;
}

enum class Alignment : int
{
	left,
	center,
	right
};

[[nodiscard]] inline auto fill_align(std::string const& str, size_t width, Alignment alignment = Alignment::left, std::u8string const& fill_char = u8" ")
	-> std::string
{
	assert(count_codepoints(fill_char) == 1);

	std::string const f = std::string(reinterpret_cast<char const*>(fill_char.data()), fill_char.size());

	auto const l = [&](size_t count) -> std::string
	{
		std::string tmp {};

		while(count-- > 0)
		{
			tmp += f;
		}
		return tmp;
	};

	size_t const len {count_codepoints(str)};
	std::string	 out {};
	if(width <= len)
	{
		out = str;
	}
	else if(alignment == Alignment::left)
	{
		out = str + l(width - len);
	}
	else if(alignment == Alignment::right)
	{
		out = l(width - len) + str;
	}
	else /*Alignment::center*/
	{
		auto const [q, r] {std::div(gsl::narrow_cast<int>(width - len), 2)};
		out = l(q + r) + str + l(q);
	}
	return out;
}

} // namespace utf
