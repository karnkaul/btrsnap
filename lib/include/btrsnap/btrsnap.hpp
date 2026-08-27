#pragma once
#include <string_view>

namespace btrsnap {
void list_snapshots();
auto take_snapshots(bool no_trim = false) -> bool;
void trim_snapshots();
auto clear_snapshots() -> bool;
void print_config_for(std::string_view subvolume);
} // namespace btrsnap
