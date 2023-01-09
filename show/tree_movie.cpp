#include "tmdb.h"
#include "tree_util.h"

#include <string>

namespace tree
{
void Movie_info::func()
{
	using namespace std::string_literals;
	auto const [query] {Param::get_arguments<arguments>()};
	auto		json = tmdb::factory<tmdb::Movie_search>(query);
	auto const& j	 = json.data;

	size_t max = std::ranges::max(j["results"] | std::views::transform([](auto const& elem) -> size_t { return elem["title"].get<std::string>().size(); }));

	

	std::format_to(std::ostream_iterator<char> {std::cout}, "{} results for \"{}\"\n", j["results"].size(), query);

	for(auto const& i : j["results"])
	{
		auto const& title = i.value("title", "-");
		auto const& date  = i.value("release_date", "-");
		auto const& id	  = i.value("id", 0);

		std::vformat_to(std::ostream_iterator<char> {std::cout}, "{:<"s + std::to_string(max) + "} {} {:>8}\n"s, std::make_format_args(title, date, id));
	}
};

void Movie_by_id::func()
{
	namespace fs = std::filesystem;
	using P		 = fs::path;

	auto [id, file]	 = Param::get_arguments<arguments>();
	auto		json = tmdb::factory<tmdb::Movie>(id);
	auto const& j	 = json.data;

	auto const& title	= j.value("title", "missing");
	auto const& date	= j.value("release_date", "missing");
	auto const& imdb_id = j.value("imdb_id", "missing");

	std::string plex_format = std::format("{} ({}) {{imdb-{}}}{}", title, date, imdb_id, file.extension().string());
	util::make_ntfs_compliant(plex_format);

	P const old_path = file;
	P const new_path = file.replace_filename(plex_format);

	std::error_code ec {};
	fs::rename(old_path, new_path, ec);
	if(ec)
	{
		std::cout << ec.message();
	}
}

void Movie_by_name::func()
{
	namespace fs = std::filesystem;
	using P		 = fs::path;

	auto [query, file] = Param::get_arguments<arguments>();
	auto const	json   = tmdb::factory<tmdb::Movie_search>(query);
	auto const& j	   = json.data;

	size_t num_movies = j["results"].size();
	std::format_to(std::ostream_iterator<char> {std::cout}, "{} results for \"{}\"\n", num_movies, query);
	if(num_movies == 0)
	{
		std::cout << lang::NO_RESULT;
		std::exit(EXIT_SUCCESS);
	}

	nlohmann::json::number_integer_t id = j["results"].at(0)["id"];

	auto const	movie	= tmdb::factory<tmdb::Movie>(id);
	auto const& m		= movie.data;
	auto const& title	= m.value("title", "missing");
	auto const& date	= m.value("release_date", "missing");
	auto const& imdb_id = m.value("imdb_id", "missing");

	std::string plex_format = std::format("{} ({}) {{imdb-{}}}{}", title, date, imdb_id, file.extension().string());
	util::make_ntfs_compliant(plex_format);

	P const old_path = file;
	P const new_path = file.replace_filename(plex_format);

	std::error_code ec {};
	fs::rename(old_path, new_path, ec);
	if(ec)
	{
		std::cout << ec.message();
	}
}

} // namespace tree