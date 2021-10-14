#include "tree_util.h"
#include "tmdb.h"
#include "util.h"

#include <vector>
#include <tuple>
#include <algorithm>
#include <ranges>
#include <cassert>


void tree::Series_by_id::func()
{
	
	assert(num_of_arguments {}() == g::ARGUMENTS->num_of_arguments_left());
	namespace fs = std::filesystem;
	using P = fs::path;

	

	auto const id{ Param::number() };
	auto const season{ Param::number() };
	auto const folder{ Param::dir() };

	
	//tmdb::Series series{ id };
	auto series{ tmdb::factory<tmdb::Series>(id) };

	//tmdb::TV tv{ id,season };
	auto tv{ tmdb::factory<tmdb::TV>(id,season) };

	

	using Ep = fs::path;
	std::vector<Ep> eps{};


	auto const digits_req{ std::max(util::count_digits(tv.episodes.size()), size_t{ 2 }) };

	auto const proj = [](auto const& e) { return e.path(); };
	auto const filter = [](auto const& e) { return fs::is_regular_file(e); };
	std::ranges::copy_if(fs::directory_iterator{ folder }, std::back_inserter(eps), filter, proj);
	std::ranges::sort(eps); //alphanumeric


	
	if (eps.size() != tv.episodes.size())
	{
		std::cout << "no, match!";
		return;

	};

	for (size_t i{}; i < eps.size(); ++i)
	{
		std::string fmt{ std::format("{{}} - s{{:0>{0}}}e{{:0>{0}}} - {{}}{1}", digits_req,eps.at(i).extension().string()) };
		std::string plex_name{ std::format(fmt, series.name, tv.season_number, tv.episodes.at(i).episode_number, tv.episodes.at(i).name) };
		util::make_ntfs_compliant(plex_name);
		
		std::cout << eps.at(i).string() << " -> \"" << plex_name << "\"\n";


		P const old_path = eps.at(i);
		P const new_path = P{ old_path }.replace_filename(plex_name);

		//if (/*dry_flag*/)
		{
			std::error_code ec{};
			fs::rename(old_path, new_path, ec);
			if(ec) { std::cout << ec.message(); }
		}
	}


	





}

void tree::Series_info::func()
{
	auto const q{ Param::string() };
	//tmdb::Series_search s{ std::string{q} };


	auto const s{ tmdb::factory<tmdb::Series_search>( std::string{q}) };
	

	if (s.m_series.empty())
	{
		std::cout << lang::NO_RESULT;
		std::exit(EXIT_SUCCESS);
	}



	using T = tmdb::Series_search::Entry const&;
	auto const largest_title = std::max_element(
		s.m_series.begin(),
		s.m_series.end(),
		[](T lhs, T rhs) noexcept -> bool {return lhs.name.size() < rhs.name.size(); });

	auto const largest_id = std::max_element(
		s.m_series.begin(),
		s.m_series.end(),
		[](T lhs, T rhs) noexcept -> bool {return util::count_digits(lhs.id) < util::count_digits(rhs.id); });


	size_t const title{ std::max(std::size_t{10},largest_title->name.size() + 1) };
	size_t const id{ std::max(util::count_digits(largest_id->id) + 1,(size_t)8) };
	size_t constexpr date{ 11 };

	//head
	std::cout << std::left
		<< std::setw(title) << lang::TITLE << std::setw(1) << '|'
		<< std::setw(date) << lang::DATE << std::setw(1) << '|'
		<< std::setw(id) << " tmdb-ID" << std::setw(0) << '\n';
	std::cout << std::setfill('-')
		<< std::setw(title) << "" << std::setw(1) << '|'
		<< std::setw(date) << "" << std::setw(1) << '|'
		<< std::setw(id) << "" << std::setw(0) << '\n';


	for (auto const& e : s.m_series)
	{
		std::cout << std::setfill(' ')
			<< std::setw(title) << e.name << std::setw(1) << '|'
			<< std::setw(date) << e.first_air_date << std::setw(1) << '|'
			<< std::right
			<< std::setw(id) << e.id << std::setw(0) << '\n'
			<< std::left;
	}
	std::cout << std::endl;

}