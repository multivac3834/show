#pragma once
#include "lang.h"
#include "util.h"

#include <array>
#include <tuple>

#include <filesystem>
#include <iostream>
#include <ranges>
#include <string_view>

namespace Param
{
struct Number
{
	static auto get_desc_short() -> std::string
	{
		return "Number";
	};

	static auto get_value(std::string_view v) -> long long
	{
		long long n {};

		auto [ptr, ec] {std::from_chars(v.data(), v.data() + v.size(), n)};

		if(ec != std::errc {})
		{
			std::cout << std::format("[{}] {}", v, std::make_error_condition(ec).message()) << std::endl;
			std::exit(EXIT_FAILURE);
		};
		return n;
	}
};

struct File
{
	static auto get_desc_short() -> std::string
	{
		return "File";
	};

	static auto get_value(std::string_view v) -> std::filesystem::path
	{
		std::error_code		  ec {};
		std::filesystem::path file {v};

		bool const is_reg_file {std::filesystem::is_regular_file(file, ec)};

		if(ec)
		{
			std::cout << ec.message();
			std::exit(EXIT_FAILURE);
		}

		if(!is_reg_file)
		{
			std::cout << lang::EXPECTED_FILE;
			std::exit(EXIT_FAILURE);
		}

		return file;
	};
};

struct Dir
{
	static auto get_desc_short() -> std::filesystem::path
	{
		return "Directory";
	};
	static auto get_value(std::string_view v) -> std::filesystem::path
	{
		std::error_code		  ec {};
		std::filesystem::path file {v};

		bool const is_reg_dir {std::filesystem::is_directory(file, ec)};

		if(ec)
		{
			std::cout << ec.message();
			std::exit(EXIT_FAILURE);
		}

		if(!is_reg_dir)
		{
			std::cout << lang::EXPECTED_DIR;
			std::exit(EXIT_FAILURE);
		}

		return file;
	};
};

struct String
{
	static auto get_desc_short() -> std::string
	{
		return "String";
	};
	static auto get_value(std::string_view v) noexcept -> std::string_view
	{
		return {v};
	};
};

template <class Tup>
auto get_arguments()
{
	Tup		   tup {};
	auto const idx_seq = std::make_index_sequence<std::tuple_size_v<Tup>> {};

	std::array<std::string_view, std::tuple_size_v<Tup>> args {};
	std::ranges::copy(g::ARGUMENTS->cmd_line, args.begin());

	auto r = [&]<std::size_t... I>(std::index_sequence<I...>)
	{
		return std::make_tuple(std::get<(I)>(tup).get_value(std::get<I>(args))...);
	}
	(idx_seq);

	

	return r;
}

template <class T>
auto print_arguments_req() -> void
{
	[]<int... n>(std::index_sequence<n...>)
	{
		((std::cout << '[' << std::tuple_element_t<n, T>::get_desc_short() << ']'), ...);
	}
	(std::make_index_sequence<std::tuple_size_v<T>> {});
}

} // namespace Param

namespace tree
{
using leaf_t = std::tuple<>;

template <class T>
auto constexpr is_leaf() -> bool
{
	return std::is_same<T::children, tree::leaf_t>::value;
}

template <class Node, int depth = 0>
void print_tree_r()
{
	for(int i = depth; i >= 0; --i)
		std::cout << "  ";

	std::cout << Node::NAME;

	if constexpr(is_leaf<Node>())
		std::cout << ' ', Param::print_arguments_req<typename Node::arguments>();

	std::cout << '\n';
	constexpr size_t num_children = std::tuple_size_v<typename Node::children>;

	if constexpr(num_children > 0)
		[&]<class T, T... n>(std::integer_sequence<T, n...>)
		{
			((print_tree_r<std::tuple_element_t<n, typename Node::children>, depth + 1>()), ...);
		}
	(std::make_index_sequence<num_children> {});
}

template <class Tup, class T, T... n>
constexpr auto get_child_index(std::integer_sequence<T, n...>, std::string_view key) -> size_t
{
	auto const count_matching_chars = [](auto const& a, auto const& b) -> size_t
	{
		auto [in1, in2] {std::ranges::mismatch(a, b)};
		if(a.size() < b.size())
		{
			return 0;
		}
		auto const d {std::distance(std::begin(a), in1)};
		return d;
	};

	std::array<std::pair<int, int>, sizeof...(n)> distances {};
#pragma warning(suppress : 26446)
	((distances[n] = std::pair<int, int> {n, count_matching_chars(std::tuple_element_t<n, Tup>::NAME, key)}), ...);
	auto const candidates = std::ranges::partition(
		distances, [](auto const& num) { return num == 0; }, &std::pair<int, int>::second);

	if(candidates.empty())
	{
		return std::numeric_limits<size_t>::max();
	}

	std::ranges::sort(candidates, std::ranges::less {}, &std::pair<int, int>::second);
	auto const best_candidate {candidates.back()};

	//ambiguous?
	return std::ranges::count(candidates, best_candidate.second, &std::pair<int, int>::second) == 1 ? best_candidate.first : std::numeric_limits<size_t>::max();
}

template <class Tup, class T, T... n>
void call_next_node(std::integer_sequence<T, n...>, size_t next)
{
	(
		[&]<class U>(auto idx)
		{
			if(idx == next)
				traverse<U>();
		}.
		operator()<typename std::tuple_element_t<n, Tup>>(n),
		...);
}

template <class Node>
void traverse()
{
	if constexpr(tree::is_leaf<Node>())
	{
		if(g::ARGUMENTS->num_of_arguments_left() == std::tuple_size_v<typename Node::arguments>)
		{
			Node::func();
		}
		else
		{
			std::cout << Node::USAGE << std::endl;
		}
	}
	else
	{
		if(g::ARGUMENTS->num_of_arguments_left() > 0)
		{
			auto constexpr idx_sq = std::make_index_sequence<std::tuple_size_v<typename Node::children>> {};
			size_t const c_idx	  = get_child_index<typename Node::children>(idx_sq, g::ARGUMENTS->pop_front());
			if(c_idx == std::numeric_limits<size_t>::max())
				tree::print_tree_r<Node>(); // non matching argument
			else
				call_next_node<typename Node::children>(idx_sq, c_idx);
		}
		else
		{
			tree::print_tree_r<Node>(); // not enough arguments
		}
	}
};

struct Movie_info
{
	using children							= leaf_t;
	using arguments							= std::tuple<Param::String>;
	constexpr static std::string_view NAME	= "info";
	constexpr static std::string_view USAGE = "Usage:\n\tinfo [query]";
	static void						  func();
};

struct Movie_by_id
{
	using children							= leaf_t;
	using arguments							= std::tuple<Param::Number, Param::File>;
	constexpr static std::string_view NAME	= "by-id";
	constexpr static std::string_view USAGE = "Usage:\n\tby-id [tmdb-id][file]";
	static void						  func();
};

struct Movie_by_name
{
	using children							= leaf_t;
	using arguments							= std::tuple<Param::String, Param::File>;
	constexpr static std::string_view NAME	= "by-name";
	constexpr static std::string_view USAGE = "Usage:\n\tby-name [title][file]";
	static void func();
};

struct Movie
{
	using children						   = std::tuple<Movie_by_id, Movie_by_name, Movie_info>;
	constexpr static std::string_view NAME = "movie";
};

struct Series_by_id
{
	using children							= leaf_t;
	using arguments							= std::tuple<Param::Number, Param::Number, Param::Dir>;
	constexpr static std::string_view NAME	= "by-id";
	constexpr static std::string_view USAGE = "Usage:\n\tby-id [tmdb-id][season][folder]";
	static void						  func();
};

struct Series_info
{
	using children							= leaf_t;
	using arguments							= std::tuple<Param::String>;
	constexpr static std::string_view NAME	= "info";
	constexpr static std::string_view USAGE = "Usage:\n\tinfo [query]";
	static void						  func();
};

struct Series
{
	using children						   = std::tuple<Series_info, Series_by_id>;
	constexpr static std::string_view NAME = "series";
};

struct Set
{
	using children							= leaf_t;
	using arguments							= std::tuple<Param::String, Param::String>;
	constexpr static std::string_view USAGE = "Usage:\n\tset [key][value]";
	constexpr static std::string_view NAME	= "set";
	static void						  func();
};

struct Get
{
	using children							= leaf_t;
	using arguments							= std::tuple<Param::String>;
	constexpr static std::string_view USAGE = "Usage:\n\tget [key]";
	constexpr static std::string_view NAME	= "get";
	static void						  func();
};

struct Options
{
	using children						   = std::tuple<Set, Get>;
	constexpr static std::string_view NAME = "options";
};

struct Root
{
	using children								 = std::tuple<Movie, Series, Options>;
	constexpr static const std::string_view NAME = "show";
};

} // namespace tree