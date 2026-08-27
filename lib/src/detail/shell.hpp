#pragma once
#include "klib/string/c_string.hpp"

namespace btrsnap::detail::shell {
auto execute(klib::CString expression) -> bool;
auto execute_privileged(std::string_view expression) -> bool;
} // namespace btrsnap::detail::shell
