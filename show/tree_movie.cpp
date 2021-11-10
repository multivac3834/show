#include "tmdb.h"
#include "tree_util.h"
#include "utf_format.h"
#include "util.h"

#include <iostream>
#include <ranges>

namespace tree
{
void Movie_info::func()
{
	auto [query] {Param::get_arguments<arguments>()};
	auto const search_result {tmdb::factory<tmdb::Movie_search>(std::string {query})};

	if(search_result.m_movies.empty())
	{
		std::cout << lang::NO_RESULT;
		std::exit(EXIT_SUCCESS);
	}

	using T = tmdb::Movie_search::Entry const&;
	auto const largest_title {std::ranges::max_element(search_result.m_movies, std::less {}, [](T const& a) { return utf::count_codepoints(a.title); })};
	auto const largest_id {std::ranges::max_element(search_result.m_movies, std::less {}, [](T const& a) { return util::count_digits(a.id); })};

	size_t const len_title {std::max(size_t {10}, utf::count_codepoints(largest_title->title) + 2)};
	size_t const len_id {std::max(util::count_digits(largest_id->id) + 2, size_t {8})};
	size_t constexpr len_date {6};
	size_t constexpr len_lang {5};

	std::cout << utf::fill_align(lang::TITLE, len_title, utf::Alignment::center) << utf::to_string(u8"║");
	std::cout << utf::fill_align(lang::DATE, len_date, utf::Alignment::center) << utf::to_string(u8"║");
	std::cout << utf::fill_align(lang::ORIG_LANG, len_lang, utf::Alignment::center) << utf::to_string(u8"║");
	std::cout << utf::fill_align("tmdb-ID", len_id, utf::Alignment::center) << '\n';

	std::cout << utf::fill_align("", len_title, utf::Alignment::left, u8"═") << utf::to_string(u8"╬");
	std::cout << utf::fill_align("", len_date, utf::Alignment::left, u8"═") << utf::to_string(u8"╬");
	std::cout << utf::fill_align("", len_lang, utf::Alignment::left, u8"═") << utf::to_string(u8"╬");
	std::cout << utf::fill_align("", len_id, utf::Alignment::left, u8"═") << '\n';

	for(auto const& mov : search_result.m_movies)
	{
		std::cout << utf::fill_align(mov.title, len_title, utf::Alignment::left) << utf::to_string(u8"║");
		std::cout << utf::fill_align(util::year_from_date(mov.release_date), len_date, utf::Alignment::center) << utf::to_string(u8"║");
		std::cout << utf::fill_align(mov.original_language, len_lang, utf::Alignment::center) << utf::to_string(u8"║");
		std::cout << utf::fill_align(std::to_string(mov.id), len_id, utf::Alignment::right) << '\n';
		
		
	}
	std::cout << std::endl;
}

void Movie_by_id::func()
{
	namespace fs = std::filesystem;
	using P		 = fs::path;

	auto [id, file] = Param::get_arguments<arguments>();

	auto const movie {tmdb::factory<tmdb::Movie>(id)};

	std::string plex_style {std::format("{0} ({1}) {{imdb-{2}}}{4}", movie.title, movie.release_date, movie.imdb_id, file.extension().string())};
	util::make_ntfs_compliant(plex_style);

	P const old_path = file;
	P const new_path = file.replace_filename(plex_style);

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

	auto [query, file] = Param::get_arguments<arguments>();
	auto const search_result {tmdb::factory<tmdb::Movie_search>(std::string {query})};

	if(search_result.m_movies.empty())
	{
		std::cout << lang::NO_RESULT;
		std::exit(EXIT_SUCCESS);
	}

	tmdb::Movie const movie {tmdb::factory<tmdb::Movie>(search_result.m_movies.front().id)};

	std::string plex_style {std::format("{} ({}) {{imdb-{}}}{}", movie.title, movie.release_date, movie.imdb_id, file.extension().string())};
	util::make_ntfs_compliant(plex_style);

	fs::path const old_path {file};
	fs::path const new_path {file.replace_filename(plex_style)};

	std::error_code ec {};
	fs::rename(old_path, new_path, ec);
	if(ec)
	{
		std::cout << ec.message();
	}
}

} // namespace tree