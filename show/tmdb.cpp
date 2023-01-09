#include "tmdb.h"
#include <format>
#include "options.h"
#include "curl_wrapper.h"


std::string tmdb::Movie_search::url(std::string_view query)
{
	auto api_key	   = Config::get_instance().get_api_key();
	auto language	   = Config::get_instance().get_lang();
	auto query_encoded = curl::percent_encode(query);
	return std::format("https://api.themoviedb.org/3/search/movie?language={0}&api_key={1}&query={2}", language, api_key, query_encoded);
}

std::string tmdb::Movie::url(nlohmann::json::number_integer_t query)
{
	auto api_key	   = Config::get_instance().get_api_key();
	auto language	   = Config::get_instance().get_lang();
	return std::format("https://api.themoviedb.org/3/movie/{}?language={}&api_key={}",query,language,api_key);

}

std::string tmdb::Tv_search::url(std::string_view query)
{
	auto api_key  = Config::get_instance().get_api_key();
	auto language = Config::get_instance().get_lang();
	auto query_encoded = curl::percent_encode(query);
	return std::format("https://api.themoviedb.org/3/search/tv?query={}&language={}&api_key={}", query_encoded, language, api_key);
}

std::string tmdb::Tv::url(nlohmann::json::number_integer_t query)
{
	auto api_key  = Config::get_instance().get_api_key();
	auto language = Config::get_instance().get_lang();
	return std::format("https://api.themoviedb.org/3/tv/{}?language={}&api_key={}", query, language, api_key);
}

std::string tmdb::Tv_season::url(nlohmann::json::number_integer_t tv_id, nlohmann::json::number_integer_t season_number)
{
	auto api_key  = Config::get_instance().get_api_key();
	auto language = Config::get_instance().get_lang();
	return std::format("https://api.themoviedb.org/3/tv/{}/season/{}?language={}&api_key={}", tv_id, season_number, language, api_key);
}
