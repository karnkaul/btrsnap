#pragma once
#include "klib/string/c_string.hpp"
#include <string_view>

namespace btrsnap {
void list_snapshots(klib::CString custom_config_path = {});
auto take_snapshots(bool no_trim = false, klib::CString custom_config_path = {}) -> bool;
void trim_snapshots(klib::CString custom_config_path = {});
auto clear_snapshots(klib::CString custom_config_path = {}) -> bool;
void print_config_for(std::string_view subvolume);
} // namespace btrsnap
