#include "curl_wrapper.h"
#include <algorithm>
#include <array>
#include <cassert>
#include <charconv>
#include <iostream>
#include <iterator>
#include <memory>
#include <span>


#include <gsl/gsl-lite.hpp>
#include <curl/curl.h>






#ifdef CURL_WIN32
constexpr bool WINDOWS {true};
#elif
constexpr bool WINDOWS {false};
#endif

namespace curl
{
struct Init
{
	Init(const Init&) = delete;					   // copy constructor
	Init(Init&&)	  = delete;					   // move constructor
	auto operator=(const Init&) -> Init& = delete; // copy assignment
	auto operator=(Init&&) -> Init& = delete;	   // move assignment

	Init() noexcept
	{
		if constexpr(WINDOWS)
		{
			SetConsoleOutputCP(CP_UTF8);
		}
#pragma warning( suppress : 26812)
		if(auto const cc = curl_global_init(CURL_GLOBAL_ALL); cc != 0)
		{
			std::exit(EXIT_FAILURE);
		}
	}

	~Init() noexcept
	{
		curl_global_cleanup();
	}
} _ {};

auto percent_encode(std::string_view const unescaped) -> std::string
{
	constexpr size_t mb_8 {1024 * 1024 * 8};
	assert(unescaped.size() < mb_8);

	std::unique_ptr<CURL, void (*)(CURL*)> curl_ctx(curl_easy_init(), curl_easy_cleanup);
	if(!curl_ctx)
	{
		std::exit(EXIT_FAILURE);
	}

	std::unique_ptr<char, void (*)(void*)> curl_string(curl_easy_escape(curl_ctx.get(), unescaped.data(), gsl::narrow_cast<int>(unescaped.length())), curl_free);
	if(!curl_string)
	{
		std::exit(EXIT_FAILURE);
	}

	return {curl_string.get()};
}

auto write_function(char* ptr, size_t size, size_t nmemb, void* userdata) -> size_t
{
	assert(userdata);
#pragma warning(suppress : 26429)
	auto*		 buffer	   = static_cast<Http_response::Buffer*>(userdata);
	size_t const real_size = size * nmemb;
#pragma warning(suppress : 26481)
	std::copy(ptr, ptr + real_size, std::back_inserter(*buffer));
	return real_size;
};

auto http_get(std::string const& url) -> Http_response
{
	assert(!url.empty());
	Http_response response {};

	struct
	{
		std::array<char, CURL_ERROR_SIZE> buffer {};
										  operator char*() noexcept
		{
			return buffer.data();
		}
		void operator=(CURLcode ec)
		{
			if(ec != CURLE_OK)
			{
				std::cout << (buffer.data() != nullptr ? std::string(buffer.data()) : "Curl error, but error buffer is empty");
				std::exit(EXIT_FAILURE);
			}
		}
	} ok; // release after curl_ctx

	std::unique_ptr<CURL, void (*)(CURL*)> curl_ctx(curl_easy_init(), curl_easy_cleanup);
	assert(curl_ctx.get());
	curl_easy_setopt(curl_ctx.get(), CURLOPT_ERRORBUFFER, (char*)ok);
	ok = curl_easy_setopt(curl_ctx.get(), CURLOPT_HEADERFUNCTION, write_function);
	ok = curl_easy_setopt(curl_ctx.get(), CURLOPT_HEADERDATA, &(response.head));
	ok = curl_easy_setopt(curl_ctx.get(), CURLOPT_WRITEFUNCTION, write_function);
	ok = curl_easy_setopt(curl_ctx.get(), CURLOPT_WRITEDATA, &(response.body));
	ok = curl_easy_setopt(curl_ctx.get(), CURLOPT_URL, url.c_str());
	ok = curl_easy_perform(curl_ctx.get());

	return response;
};

Http_response::operator std::string_view() const noexcept
{
	return {body.data(), body.size()};
}

auto Http_response::get_status_code() const noexcept -> int
{
	std::string_view header {head.data(), head.size()};

	size_t const pos_sc {header.find_first_of(' ') + 1};

	if(pos_sc == std::string_view::npos)
	{
		return -1;
	}
	header.remove_prefix(pos_sc);

	int n {};

	auto [ptr, ec] {std::from_chars(header.data(), &header.back(), n)};
	return ec == std::errc {} ? n : -1;
}

} // namespace curl
