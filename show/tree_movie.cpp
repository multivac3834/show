#include "tmdb.h"
#include "tree_util.h"

#include <string>

namespace tree
{

using namespace std::string_literals;
using namespace std::string_view_literals;

void Movie_info::func()
{
	auto const& [query] {Param::get_arguments<arguments>()};
	nlohmann::json movies = tmdb::factory<tmdb::Movie_search>(query).data;

	if(movies.at("results").empty())
	{
		std::format_to(std::ostream_iterator<char> {std::cout}, "No movies found for \"{}\".\n", query);
		return;
	}

	size_t const max =
		std::ranges::max(movies.at("results") | std::views::transform([](auto const& elem) -> size_t { return elem.value("title", "%title%"s).size(); }));

	std::format_to(std::ostream_iterator<char> {std::cout}, "{} results for \"{}\"\n", movies.at("results").size(), query);

	for(auto const& movie : movies["results"])
	{
		std::string		   title	  = movie.value("title", "%title%"s);
		std::string		   date		  = movie.value("release_date", "%release_date%"s);
		unsigned long long identifier = movie.value("id", 0ULL);

		std::vformat_to(std::ostream_iterator<char> {std::cout}, "{:<"s + std::to_string(max) + "} {} {:>8}\n"s,
						std::make_format_args(title, date, identifier));
	}
};

void rename_movie(std::filesystem::path const& file, nlohmann::json const& movie)
{
	using namespace std::string_literals;
	using namespace std::filesystem;
	std::string title = movie.value("title", "%title%"s);
	std::string date  = movie.value("release_date", "0000"s);
	if(date.length() > 4)
	{
		date = date.substr(0, 4);
	}
	std::string imdb_id		= movie.value("imdb_id", "%imdb_id%"s);
	std::string plex_format = std::format("{} ({}) {{imdb-{}}}{}", title, date, imdb_id, file.extension().string());
	util::make_ntfs_compliant(plex_format);

	path const new_path = path {file}.replace_filename(plex_format);

	std::error_code error_code {};
	rename(file, new_path, error_code);
	if(error_code)
	{
		std::cout << error_code.message();
	}
}

void Movie_by_id::func()
{
	using namespace std::string_literals;
	using namespace std::filesystem;
	auto [id, file]		 = Param::get_arguments<arguments>();
	nlohmann::json movie = tmdb::factory<tmdb::Movie>(id).data;
	rename_movie(file, movie);
}

void Movie_by_name::func()
{
	using namespace std::string_literals;
	using namespace std::filesystem;

	auto [query, file]	  = Param::get_arguments<arguments>();
	nlohmann::json movies = tmdb::factory<tmdb::Movie_search>(query).data;

	size_t num_movies = movies.at("results").size();
	std::format_to(std::ostream_iterator<char> {std::cout}, "{} results for \"{}\"\n", num_movies, query);
	if(num_movies == 0)
	{
		std::cout << lang::NO_RESULT;
		std::exit(EXIT_SUCCESS);
	}

	nlohmann::json::number_integer_t best_movie = movies["results"].at(0)["id"];
	nlohmann::json					 movie		= tmdb::factory<tmdb::Movie>(best_movie).data;

	rename_movie(file, movie);
}

} // namespace tree