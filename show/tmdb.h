#pragma once
#include "cache.h"
#include "curl_wrapper.h"
#include <nlohmann/json.hpp>
#include <string_view>

namespace tmdb
{

struct Movie_search
{
	static auto	   url(std::string_view query) -> std::string;
	nlohmann::json data;
};

struct Movie
{
	static auto	   url(nlohmann::json::number_integer_t query) -> std::string;
	nlohmann::json data;
};

struct Tv_search
{
	static auto	   url(std::string_view query) -> std::string;
	nlohmann::json data;
};

struct Tv
{
	static auto	   url(nlohmann::json::number_integer_t query) -> std::string;
	nlohmann::json data;
};

struct Tv_season
{
	static auto	   url(nlohmann::json::number_integer_t tv_id, nlohmann::json::number_integer_t season_number) -> std::string;
	nlohmann::json data;
};

template <typename T, typename... U>
auto factory(U... query) -> T
{
	T			obj {};
	std::string url = obj.url(query...);
	if(cache::is_stored(url))
	{
		std::string result = cache::get(url);
		obj.data		   = nlohmann::json::parse(result);
	}
	else
	{
		auto const result {curl::http_get(url)};
		int const  return_code = result.get_status_code();

		switch(return_code)
		{
		default:
			throw static_cast<int>(return_code);

		case curl::Http_status_code::ok:
			break;
		}

		cache::add(cache::Add_parameter {.json = result, .url = url});
		obj.data = nlohmann::json::parse(std::string_view {result});
	}

	return obj;
};

} // namespace tmdb
