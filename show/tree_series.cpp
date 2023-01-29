#include "tmdb.h"
#include "tree_util.h"
#include "util.h"

#include <filesystem>
#include <format>
#include <iterator>
#include <vector>

void tree::Series_by_id::func()
{
	auto [show_id, season_num, folder] {Param::get_arguments<arguments>()};

	auto const& show	 = tmdb::factory<tmdb::Tv>(show_id).data;
	auto const& season	 = tmdb::factory<tmdb::Tv_season>(show_id, season_num).data;
	auto const& episodes = season.at("episodes");
	std::format_to(std::ostream_iterator<char> {std::cout}, "Tv show \"{}\" found.\n", show.value("name", "%name%"));
	std::format_to(std::ostream_iterator<char> {std::cout}, "Season {} has {} episodes.\n", season_num, episodes.size());

	using namespace std::filesystem;
	auto			  filter = [](directory_entry const& file) -> bool { return file.is_regular_file(); };
	std::vector<path> files {};
	std::ranges::copy_if(recursive_directory_iterator {folder}, std::back_inserter(files), filter, {});
	std::ranges::sort(files);

	if(files.size() != episodes.size())
	{
		std::cout << "mismatch!!!\n";
		std::exit(EXIT_FAILURE);
	}

	for(size_t i = 0; i < files.size() && i < episodes.size(); ++i)
	{
		std::string wish = std::format("{} - s{:0>2}e{:0>2} - {}", show["name"].get<std::string>(), season_num, i + 1, episodes[i]["name"].get<std::string>());
		util::make_ntfs_compliant(wish);

		path new_path {folder};
		new_path /= wish;
		new_path += files[i].extension();

		std::cout << files[i] << '\n';
		std::cout << wish << '\n';

		rename(files[i], new_path);
	}
}

void tree::Series_info::func()
{
	using namespace std::string_literals;
	auto [query] {Param::get_arguments<arguments>()};
	nlohmann::json const show = tmdb::factory<tmdb::Tv_search>(query).data;

	if(!show.contains("results") || show.at("results").empty())
	{
		std::cout << "no results";
		return;
	}
	size_t max = std::ranges::max(show.at("results") | std::views::transform([](auto const& elem) -> size_t { return elem.value("name", ""s).size(); }));

	for(auto const& episode : show["results"])
	{
		nlohmann::json::string_t const		   name		  = episode.value("name", "%name%"s);
		nlohmann::json::number_integer_t const identifier = episode.value("id", 0LL);
		nlohmann::json::string_t const		   date		  = episode.value("first_air_date", "%first_air_date%");
		std::vformat_to(std::ostream_iterator<char> {std::cout}, "{:<"s + std::to_string(max) + "} {:10} {:>9}\n"s,
						std::make_format_args(name, !date.empty() ? date : "0000-00-00", identifier));
	}
}