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
};

struct File
{
   static auto get_desc_short() -> std::string
   {
	  return "File";
   };
};

struct Dir
{
   static auto get_desc_short() -> std::string
   {
	  return "Directory";
   };
};

struct String
{
   static auto get_desc_short() -> std::string
   {
	  return "String";
   };
};

template <class T>
auto print_arguments_req() -> void
{
   []<int... n>(std::index_sequence<n...>)
   {
	  ((std::cout << '[' << std::tuple_element_t<n, T>::get_desc_short() << ']'), ...);
   }
   (std::make_index_sequence<std::tuple_size_v<T>> {});
}

template <class T>
auto get_arguments()
{
   return std::make_tuple(1, 2, 3);
}

[[nodiscard]] inline auto number() -> __int64
{
   auto const v {g::ARGUMENTS->pop_front()};
   __int64	  n {};

#pragma warning(suppress : 26481)
   auto [ptr, ec] {std::from_chars(v.data(), v.data() + v.size(), n)};

   if(ec != std::errc {})
   {
	  std::cout << std::format("[{}] {}", v, std::make_error_condition(ec).message()) << std::endl;
	  std::exit(EXIT_FAILURE);
   };

   return n;
};

[[nodiscard]] inline auto file() -> std::filesystem::path
{
   auto const			 v {g::ARGUMENTS->pop_front()};
   std::error_code		 ec {};
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

[[nodiscard]] inline auto dir() -> std::filesystem::path
{
   std::string_view const v {g::ARGUMENTS->pop_front()};

   std::error_code		 ec {};
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

[[nodiscard]] inline auto string() noexcept -> std::string_view
{
   return {g::ARGUMENTS->pop_front()};
}

} // namespace Param

namespace tree
{
using leaf_t = std::tuple<>;

template <size_t n>
struct Number
{
   constexpr auto operator()() noexcept -> size_t
   {
	  return n;
   };
};

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
	  std::cout << ' ', Param::print_arguments_req<Node::arguments>();

   std::cout << '\n';
   constexpr size_t num_children = std::tuple_size_v<Node::children>;

   if constexpr(num_children > 0)
	  [&]<class T, T... n>(std::integer_sequence<T, n...>)
	  {
		 ((print_tree_r<std::tuple_element_t<n, Node::children>, depth + 1>()), ...);
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
	   operator()<std::tuple_element_t<n, Tup>>(n),
	   ...);
}

template <class Node>
void traverse()
{
   if constexpr(tree::is_leaf<Node>())
   {
	  if(g::ARGUMENTS->num_of_arguments_left() == Node::num_of_arguments()())
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
		 auto constexpr idx_sq = std::make_index_sequence<std::tuple_size_v<Node::children>> {};
		 size_t const c_idx	   = get_child_index<typename Node::children>(idx_sq, g::ARGUMENTS->pop_front());
		 if(c_idx == std::numeric_limits<size_t>::max())
			tree::print_tree_r<Node>(); // non matching argument
		 else
			call_next_node<Node::children>(idx_sq, c_idx);
	  }
	  else
	  {
		 tree::print_tree_r<Node>(); // not enough arguments
	  }
   }
};

struct Movie_info
{
   using children						   = leaf_t;
   using num_of_arguments				   = tree::Number<1>;
   using arguments						   = std::tuple<Param::String>;
   constexpr static std::string_view NAME  = "info";
   constexpr static std::string_view USAGE = "Usage:\n\tinfo [query]";
   static void						 func();
};

struct Movie_by_id
{
   using children						   = leaf_t;
   using num_of_arguments				   = tree::Number<2>;
   using arguments						   = std::tuple<Param::Number, Param::File>;
   constexpr static std::string_view NAME  = "by-id";
   constexpr static std::string_view USAGE = "Usage:\n\tby-id [tmdb-id][file]";
   static void						 func();
};

struct Movie_by_name
{
   using children						   = leaf_t;
   using num_of_arguments				   = tree::Number<2>;
   using arguments						   = std::tuple<Param::String, Param::File>;
   constexpr static std::string_view NAME  = "by-name";
   constexpr static std::string_view USAGE = "Usage:\n\tby-name [title][file]";

   static void func();
};

struct Movie
{
   using children						  = std::tuple<Movie_by_id, Movie_by_name, Movie_info>;
   constexpr static std::string_view NAME = "movie";
};

struct Series_by_id
{
   using children						   = leaf_t;
   using num_of_arguments				   = tree::Number<3>;
   using arguments						   = std::tuple<Param::Number, Param::Number, Param::Dir>;
   constexpr static std::string_view NAME  = "by-id";
   constexpr static std::string_view USAGE = "Usage:\n\tby-id [tmdb-id][season][folder]";
   static void						 func();
};

struct Series_info
{
   using children						   = leaf_t;
   using num_of_arguments				   = tree::Number<1>;
   using arguments						   = std::tuple<Param::String>;
   constexpr static std::string_view NAME  = "info";
   constexpr static std::string_view USAGE = "Usage:\n\tinfo [query]";
   static void						 func();
};

struct Series
{
   using children						  = std::tuple<Series_info, Series_by_id>;
   constexpr static std::string_view NAME = "series";
};

struct Set
{
   using children						   = leaf_t;
   using num_of_arguments				   = tree::Number<2>;
   using arguments						   = std::tuple<Param::String, Param::String>;
   constexpr static std::string_view USAGE = "Usage:\n\tset [key][value]";
   constexpr static std::string_view NAME  = "set";
   static void						 func();
};

struct Get
{
   using children						   = leaf_t;
   using num_of_arguments				   = tree::Number<1>;
   using arguments						   = std::tuple<Param::String>;
   constexpr static std::string_view USAGE = "Usage:\n\tget [key]";
   constexpr static std::string_view NAME  = "get";
   static void						 func();
};

struct Options
{
   using children						  = std::tuple<Set, Get>;
   constexpr static std::string_view NAME = "options";
};

struct Root
{
   using children								= std::tuple<Movie, Series, Options>;
   constexpr static const std::string_view NAME = "show";
};

} // namespace tree