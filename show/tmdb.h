#pragma once
#include "cache.h"
#include "curl_wrapper.h"
#include <nlohmann/json.hpp>
#include <string_view>

namespace tmdb
{

struct Movie_search
{
	static std::string url(std::string_view query);
	nlohmann::json	   data;
};

struct Movie
{
	static std::string url(nlohmann::json::number_integer_t query);
	nlohmann::json	   data;
};


struct Tv_search
{
	static std::string url(std::string_view query);
	nlohmann::json	   data;
};

struct Tv
{
	static std::string url(nlohmann::json::number_integer_t query);
	nlohmann::json	   data;
};

struct Tv_season
{
	static std::string url(nlohmann::json::number_integer_t tv_id, nlohmann::json::number_integer_t season_number);
	nlohmann::json	   data;
};



template <typename T,typename ... U>
T factory(U ... query)
{
	std::string url = T::url(query...);
	if(cache::is_stored(url))
	{
		std::string result = cache::get(url);
		return T {.data = nlohmann::json::parse(result)};
	}

	else
	{
		auto const result {curl::http_get(url)};
		int const  return_code = result.get_status_code();

		switch(return_code)
		{
		default:
		case 404: 
		case 401:			
			throw return_code;

		case 200:
			break;
		}
		cache::add(result, url);
		return T {.data = nlohmann::json::parse(std::string_view {result})};
	}
}

} // namespace tmdb

