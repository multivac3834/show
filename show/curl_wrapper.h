#pragma once
#include <string>
#include <string_view>
#include <vector>

namespace curl
{

enum Http_status_code : int
{
	ok = 200
};

struct Http_response
{
	using Buffer = std::vector<char>;
	Buffer head;
	Buffer body;

	[[nodiscard]]	   operator std::string_view() const noexcept;
	[[nodiscard]] auto get_status_code() const noexcept -> int;
};

[[nodiscard]] auto percent_encode(std::string_view unescaped) -> std::string;
[[nodiscard]] auto http_get(std::string const& url) -> Http_response;

} // namespace curl