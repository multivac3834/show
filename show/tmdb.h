#pragma once

#include <format>
#include <iostream>
#include <string>
#include <variant>
#include <vector>

#define BOOST_JSON_STANDALONE
#include <boost/json.hpp>

#include "cache.h"
#include "curl_wrapper.h"

namespace tmdb
{
using integer = __int64;
using number  = double;
using boolean = bool;
using string  = std::string;

template <typename T>
concept IsJsonType = (std::same_as<T, integer> || std::same_as<T, number> || std::same_as<T, boolean> || std::same_as<T, string>);

template <class T>
concept isApiResult = requires(T x)
{
	{
		x.url
		} -> std::convertible_to<std::string>;
	{x.parse(boost::json::standalone::value {})};
};

struct Movie
{
	Movie() = delete;
	Movie(integer movie_id);

	// isApiResult
	std::string const url;
	void			  parse(boost::json::standalone::value const& json);

	std::vector<std::pair<std::string, std::string>> production_countries {};
	boolean adult {}, video {};
	string	backdrop_path {}, homepage {}, imdb_id {}, original_language {}, original_title {}, overview {}, release_date {}, tagline {}, title {};
	integer budget {}, id {}, runtime {}, vote_count {};
	number	popularity {}, vote_average {};
};

struct Movie_search
{
	struct Entry
	{
		string	overview, release_date, original_title, original_language, title;
		integer id, vote_count;
	};

	Movie_search() = delete;
	Movie_search(string const& query);

	// isApiResult
	std::string const url;
	void			  parse(boost::json::standalone::value const& json);

	std::vector<Entry> m_movies {};
};

struct Series
{
	string	name {};
	integer number_of_seasons {};
	integer id {};

	Series() = delete;
	Series(integer tv_id);

	// isApiResult
	std::string const url;
	void			  parse(boost::json::standalone::value const& json);
};

class Series_search
{
  public:
	struct Entry
	{
		string	name, first_air_date;
		integer id;
	};

	// isApiResult
	std::string const url;
	void			  parse(boost::json::standalone::value const& json);

	Series_search() = delete;
	Series_search(string const& query);

	std::vector<Entry> m_series {};
};

struct TV
{
	struct Episodes
	{
		string	air_date, name, overview, production_code, still_path;
		integer episode_number, id, season_number, vote_count;
		number	vote_average;
	};

	// isApiResult
	std::string const url;
	void			  parse(boost::json::standalone::value const& json);

	TV() = delete;
	TV(integer tv_id, integer season_number);

	string				  _id {}, air_date {}, name {}, overview {}, poster_path {};
	integer				  id {}, season_number {};
	std::vector<Episodes> episodes;
};

struct Status
{
	string	status_message {};
	integer status_code {};
	Status(boost::json::standalone::value const& json);
	operator std::string();
};

struct Error
{
	std::vector<string> errors {};
	Error(boost::json::standalone::value const& json);
	operator std::string();
};

template <isApiResult T, class... Us>
auto factory(Us const&... args) -> T
{
	using namespace boost::json;

	auto const get_json = [](auto raw_json) -> standalone::value
	{
		error_code		  ec {};
		standalone::value json = parse(raw_json, ec, storage_ptr(), {});
		assert(!ec);
		return json;
	};

	T obj {args...};

	if(cache::is_stored(obj.url))
	{
		std::string result = cache::get(obj.url);
		obj.parse(get_json(result));
		return obj;
	}
	else
	{
		auto	  r = curl::http_get(obj.url);
		int const sc {r.get_status_code()};

		auto json = get_json(r);

		switch(sc)
		{
		case 401:
		case 404:
			std::cout << std::string {Status {json}};
			break;
		case 422:
			std::cout << std::string {Error {json}};
			break;
		default:
			std::cout << std::format("unexpected HTTP {}\n\n{}", sc, std::string_view {r.head.data(), r.head.size()});
			break;
		case 200:
			cache::add(r, obj.url);
			obj.parse(json);
			return obj;
		}
	}

	std::exit(EXIT_FAILURE);
};

} // namespace tmdb