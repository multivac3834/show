#include "tmdb.h"
#include "tree_util.h"
#include "util.h"

#include <iostream>

namespace tree
{
void Movie_info::func()
{
	auto const query {Param::string()};
	auto const search_result {tmdb::factory<tmdb::Movie_search>(std::string {query})};

	if(search_result.m_movies.empty())
	{
		std::cout << lang::NO_RESULT;
		std::exit(EXIT_SUCCESS);
	}

	using T = tmdb::Movie_search::Entry const&;
	auto const largest_title =
		std::max_element(search_result.m_movies.begin(), search_result.m_movies.end(),
						 [](T lhs, T rhs) -> bool { return lhs.title.size() < rhs.title.size(); });

	auto const largest_id = std::max_element(search_result.m_movies.begin(), search_result.m_movies.end(),
											 [](T lhs, T rhs) noexcept -> bool
											 { return util::count_digits(lhs.id) < util::count_digits(rhs.id); });

	size_t const title {std::max(std::size_t {10}, largest_title->title.size() + 1)};
	size_t const id {std::max(util::count_digits(largest_id->id) + 1, (size_t)8)};
	size_t constexpr date {11};

	//head
	std::cout << std::left << std::setw(title) << lang::TITLE << std::setw(1) << '|' << std::setw(date) << lang::DATE
			  << std::setw(1) << '|' << std::setw(id) << " tmdb-ID" << std::setw(0) << '\n';
	std::cout << std::setfill('-') << std::setw(title) << "" << std::setw(1) << '|' << std::setw(date) << ""
			  << std::setw(1) << '|' << std::setw(id) << "" << std::setw(0) << '\n';

	for(auto const& e : search_result.m_movies)
	{
		std::cout << std::setfill(' ') << std::setw(title) << e.title << std::setw(1) << '|' << std::setw(date)
				  << e.release_date << std::setw(1) << '|' << std::right << std::setw(id) << e.id << std::setw(0)
				  << '\n'
				  << std::left;
	}
	std::cout << std::endl;
}

void Movie_by_id::func()
{
	namespace fs = std::filesystem;
	using P		 = fs::path;

	auto const id {Param::number()};
	auto	   file {Param::file()};

	auto const movie {tmdb::factory<tmdb::Movie>(id)};

	std::string plex_style {std::format("{} ({}) {{imdb-{}}}{}", movie.title, movie.release_date, movie.imdb_id,
										file.extension().string())};
	util::make_ntfs_compliant(plex_style);

	P const old_path = file;
	P const new_path = file.replace_filename(plex_style);

	std::error_code ec {};
	fs::rename(old_path, new_path, ec);
	if(ec) { std::cout << ec.message(); }
}

void Movie_by_name::func()
{
	namespace fs = std::filesystem;
	using P		 = fs::path;
	auto const q {Param::string()};
	auto	   file {Param::file()};

	auto const s {tmdb::factory<tmdb::Movie_search>(std::string {q})};

	if(!s.m_movies.empty())
	{
		std::cout << lang::NO_RESULT;
		std::exit(EXIT_SUCCESS);
	}

	tmdb::Movie const movie {tmdb::factory<tmdb::Movie>(s.m_movies.at(0).id)};

	std::string plex_style {std::format("{} ({}) {{imdb-{}}}{}", movie.title, movie.release_date, movie.imdb_id,
										file.extension().string())};
	util::make_ntfs_compliant(plex_style);

	P const old_path = file;
	P const new_path = file.replace_filename(plex_style);

	std::error_code ec {};
	fs::rename(old_path, new_path, ec);
	if(ec) { std::cout << ec.message(); }
}

} // namespace tree