#include "tmdb.h"
#include "tree_util.h"
#include "util.h"

#include "utf_format.h"

#include <algorithm>
#include <cassert>
#include <ranges>
#include <tuple>
#include <vector>

void tree::Series_by_id::func()
{
	namespace fs = std::filesystem;
	

	auto [id, season, folder] = Param::get_arguments<arguments>();

	
	auto series {tmdb::factory<tmdb::Series>(id)};
	auto tv {tmdb::factory<tmdb::TV>(id, season)};

	
	std::vector<fs::path> eps {};

	auto const digits_req {std::max(util::count_digits(tv.episodes.size()), size_t {2})};

	auto const proj {[](auto const& e) { return e.path(); }};
	auto const filter {[](auto const& e) { return fs::is_regular_file(e); }};
	std::ranges::copy_if(fs::directory_iterator {folder}, std::back_inserter(eps), filter, proj);
	std::ranges::sort(eps); //alphanumeric

	if(eps.size() != tv.episodes.size())
	{
		std::cout << "no, match!";
		return;
	};

	for(size_t i {}; i < eps.size(); ++i)
	{
		std::string fmt {std::format("{{}} - s{{:0>{0}}}e{{:0>{0}}} - {{}}{1}", digits_req, eps.at(i).extension().string())};
		std::string plex_name {std::format(fmt, series.name, tv.season_number, tv.episodes.at(i).episode_number, tv.episodes.at(i).name)};
		util::make_ntfs_compliant(plex_name);

		std::cout << eps.at(i).filename().string() << " -> \"" << plex_name << "\"\n";

		fs::path const old_path {eps.at(i)};
		fs::path const new_path = fs::path {old_path}.replace_filename(plex_name);

		//if (/*dry_flag*/)
		{
			std::error_code ec {};
			fs::rename(old_path, new_path, ec);
			if(ec)
			{
				std::cout << ec.message();
			}
		}
	}
}

void tree::Series_info::func()
{
	auto [q] {Param::get_arguments<arguments>()};
	auto const s {tmdb::factory<tmdb::Series_search>(std::string {q})};

	if(s.m_series.empty())
	{
		std::cout << lang::NO_RESULT;
		std::exit(EXIT_SUCCESS);
	}

	using T = tmdb::Series_search::Entry const&;
	auto const largest_title {std::ranges::max_element(s.m_series, std::less {}, [](auto const& e) { return e.name.size(); })};
	auto const largest_id {std::ranges::max_element(s.m_series, std::less {}, [](T a) { return util::count_digits(a.id); })};

	size_t const len_title {std::max(std::size_t {10}, largest_title->name.size() + 1)};
	size_t const len_id {std::max(util::count_digits(largest_id->id) + 1, (size_t)8)};
	size_t constexpr len_date {12};

	std::cout << utf::fill_align(lang::TITLE, len_title, utf::Alignment::center) << utf::to_string(u8"║");
	std::cout << utf::fill_align(lang::DATE, len_date, utf::Alignment::center) << utf::to_string(u8"║");
	std::cout << utf::fill_align("tmdb-ID", len_id, utf::Alignment::center) << "\n";

	std::cout << utf::fill_align("", len_title, utf::Alignment::left, u8"═") << utf::to_string(u8"╬");
	std::cout << utf::fill_align("", len_date, utf::Alignment::left, u8"═") << utf::to_string(u8"╬");
	std::cout << utf::fill_align("", len_id, utf::Alignment::left, u8"═") << '\n';

	for(auto const& e : s.m_series)
	{
		std::cout << utf::fill_align(e.name, len_title, utf::Alignment::left) << utf::to_string(u8"║");
		std::cout << utf::fill_align(e.first_air_date, len_date, utf::Alignment::center) << utf::to_string(u8"║");
		std::cout << utf::fill_align(std::to_string(e.id), len_id, utf::Alignment::right) << "\n";
	}
	std::cout << std::endl;
}