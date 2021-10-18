#include "util.h"
#include <cassert>

namespace g
{
std::unique_ptr<util::Args> ARGUMENTS {};
}

auto util::Args::num_of_arguments_left() const noexcept -> size_t
{
   return cmd_line.size();
}

auto util::Args::pop_front() noexcept -> std::string_view
{
   assert(cmd_line.size());
   std::string_view front = cmd_line.front();
   cmd_line				  = cmd_line.subspan(1);
   return front;
}

auto util::count_digits(__int64 n) noexcept -> size_t
{
   return std::to_string(n).length();
}

auto util::make_ntfs_compliant(std::string& in) -> void
{
   auto const bad_char = [](auto const& i)
   { return i == '/' || i == '\\' || i == ':' || i == '*' || i == '?' || i == '"' || i == '<' || i == '>' || i == '|'; };
   in.erase(std::remove_if(in.begin(), in.end(), bad_char), in.end());
}
