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
	std::format_to(std::ostream_iterator<char> {std::cout}, "Tv show \"{}\" found\n", show.value("name", "No name"));

	using namespace std::filesystem;
	auto			  filter = [](directory_entry const& file) -> bool { return file.is_regular_file(); };
	std::vector<path> files {};
	std::ranges::copy_if(recursive_directory_iterator {folder}, std::back_inserter(files), filter, {});
	std::ranges::sort(files);

	if(files.size() != episodes.size())
	{
		std::cout << "missmatch!!!";
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
	std::exit(EXIT_SUCCESS);
}

void tree::Series_info::func()
{
	auto [query] {Param::get_arguments<arguments>()};
	auto const	results = tmdb::factory<tmdb::Tv_search>(query);
	auto const& j		= results.data;

	for(auto const& i : j["results"])
	{
		nlohmann::json::string_t const&			name = i.value("name", "-");
		nlohmann::json::number_integer_t const& id	 = i.value("id", -1);
		nlohmann::json::string_t const&			date = i.value("first_air_date", "-");

		std::format_to(std::ostream_iterator<char> {std::cout}, "{} {} {}\n", name, date, id);
	}
}