
#include "tmdb.h"
#include "curl_wrapper.h"
#include "options.h"

#include <sstream>
#include <string>

#include <cassert>
#include <memory>
#include <span>

#pragma warning(push, 1)

#define BOOST_JSON_STANDALONE
#include <boost/json.hpp>
#include <boost/json/src.hpp>

#pragma warning(pop)

namespace tmdb
{
auto prefix() -> std::string
{
	return {"https://api.themoviedb.org/3/"};
}
auto key() -> std::string
{
	return {Config::get_instance().get_api_key()};
}

auto lang() -> std::string
{
	return {Config::get_instance().get_lang()};
}

template <tmdb::IsJsonType T>
auto value_or_default(boost::json::value const& root, std::string_view key, T const& default_value) -> T
{
	if constexpr(std::is_same_v<tmdb::boolean, T>)
	{
		try
		{
			return root.at(key).as_bool();
		}
		catch(...)
		{
			return default_value;
		}
	}

	if constexpr(std::is_same_v<tmdb::string, T>)
	{
		try
		{
			return tmdb::string(root.at(key).as_string());
		}
		catch(...)
		{
			return default_value;
		}
	}

	if constexpr(std::is_same_v<tmdb::integer, T>)
	{
		try
		{
			return root.at(key).as_int64();
		}
		catch(...)
		{
			return default_value;
		}
	}

	if constexpr(std::is_same_v<tmdb::number, T>)
	{
		try
		{
			return root.at(key).as_double();
		}
		catch(...)
		{
			return default_value;
		}
	}
}

Movie::Movie(integer movie_id) : url {std::format("{}movie/{}?{}&{}", prefix(), movie_id, key(), lang())}
{
}

TV::TV(integer tv_id, integer season_number_)
	: url {std::format("{}tv/{}/season/{}?{}&", prefix(), tv_id, season_number_, key(), lang())}
{
}

void Movie::parse(boost::json::standalone::value const& json)
{
	using namespace boost::json;
	using namespace std::string_literals;

	adult = value_or_default(json, "adult", false);
	video = value_or_default(json, "video", false);

	backdrop_path	  = value_or_default(json, "backdrop_path", ""s);
	homepage		  = value_or_default(json, "homepage", ""s);
	imdb_id			  = value_or_default(json, "imdb_id", ""s);
	original_language = value_or_default(json, "original_language", ""s);
	original_title	  = value_or_default(json, "original_title", ""s);
	overview		  = value_or_default(json, "overview", ""s);
	release_date	  = value_or_default(json, "release_date", ""s);
	tagline			  = value_or_default(json, "tagline", ""s);
	title			  = value_or_default(json, "title", ""s);

	budget	   = value_or_default(json, "budget", tmdb::integer {0});
	id		   = value_or_default(json, "id", tmdb::integer {0});
	runtime	   = value_or_default(json, "runtime", tmdb::integer {0});
	vote_count = value_or_default(json, "vote_count", tmdb::integer {0});

	popularity	 = value_or_default(json, "popularity", 0.0);
	vote_average = value_or_default(json, "vote_average", 0.0);

	auto const& pcs = json.at("production_countries").as_array();

	for(auto const& pc : pcs)
	{
		std::string iso_3166_1 = value_or_default(pc, "iso_3166_1", ""s);
		std::string name	   = value_or_default(pc, "name", ""s);


		//this->production_countries.emplace_back(iso_3166_1, name);
	
	}
}

void TV::parse(boost::json::standalone::value const& json)
{
	using namespace boost::json;
	using namespace std::string_literals;
	_id			= value_or_default(json, "_id", ""s);
	air_date	= value_or_default(json, "air_date", ""s);
	name		= value_or_default(json, "name", ""s);
	overview	= value_or_default(json, "overview", ""s);
	poster_path = value_or_default(json, "poster_path", ""s);

	id			  = value_or_default(json, "id", tmdb::integer {0});
	season_number = value_or_default(json, "season_number", tmdb::integer {0});
	auto const& eps	  = json.at("episodes").as_array();

	for(auto const& ep : eps)
	{
		Episodes d {};

		d.air_date		  = value_or_default(ep, "air_date", ""s);
		d.name			  = value_or_default(ep, "name", ""s);
		d.overview		  = value_or_default(ep, "overview", ""s);
		d.production_code = value_or_default(ep, "production_code", ""s);
		d.still_path	  = value_or_default(ep, "still_path", ""s);

		d.episode_number = value_or_default(ep, "episode_number", tmdb::integer {0});
		d.id			 = value_or_default(ep, "id", tmdb::integer {0});
		d.season_number	 = value_or_default(ep, "season_number", tmdb::integer {0});
		d.vote_count	 = value_or_default(ep, "vote_count", tmdb::integer {0});

		d.vote_average = value_or_default(ep, "vote_average", tmdb::number {0.0});

		this->episodes.push_back(d);
	}
}


void Movie_search::parse(boost::json::standalone::value const& json)
{
	using namespace boost::json;
	using namespace std::string_literals;

	auto const& results = json.at("results").as_array();
	for(auto const& ep : results)
	{
		Entry entry {value_or_default(ep, "overview", ""s),
					 value_or_default(ep, "release_date", ""s),
					 value_or_default(ep, "original_title", ""s),
					 value_or_default(ep, "original_language", ""s),
					 value_or_default(ep, "title", ""s),
					 value_or_default(ep, "id", tmdb::integer {0}),
					 value_or_default(ep, "vote_count", tmdb::integer {0})};

		m_movies.push_back(std::move(entry));
	}
}

Movie_search::Movie_search(string const& query)
	: url {std::format("{0}search/movie?{1}&{2}&query={3}", prefix(), key(), lang(), curl::percent_encode(query))}
{
}

Series_search::Series_search(string const& query)
	: url {std::format("{0}search/tv?{1}&{2}&query={3}", prefix(), key(), lang(), curl::percent_encode(query))}
{
}

void Series_search::parse(boost::json::standalone::value const& json)
{
	auto const& results = json.at("results").as_array();
	for(auto const& ep : results)
	{
		Entry e {};
		e.name			 = value_or_default(ep, "name", std::string {});
		e.first_air_date = value_or_default(ep, "first_air_date", std::string {});
		e.id			 = value_or_default(ep, "id", tmdb::integer {});
		m_series.push_back(e);
	}
}

Series::Series(integer tv_id) : url {std::format("{}tv/{}?{}&{}", prefix(), tv_id, key(), lang())} {};

void Series::parse(boost::json::standalone::value const& json)
{
	using namespace boost::json;
	using namespace std::string_literals;

	name			  = value_or_default(json, "name", ""s);
	number_of_seasons = value_or_default(json, "number_of_seasons", tmdb::integer {0});
	id				  = value_or_default(json, "id", tmdb::integer {0});
}

Status::Status(boost::json::standalone::value const& json)
{
	status_message = value_or_default(json, "status_message", tmdb::string {});
	status_code	   = value_or_default(json, "status_code", tmdb::integer {});
}

Status::operator std::string()
{
	return std::format("tmdb:\n{0}: {1}", this->status_code, status_message);
}

Error::Error(boost::json::standalone::value const& json)
{
	auto const& errs = json.at("errors").as_array();

	for(auto const& err : errs) { errors.emplace_back(err.as_string()); }
}

Error::operator std::string()
{
	std::string out {};
	for(auto const& e : this->errors) { out += e + '\n'; }
	return out;
}

} // namespace tmdb
