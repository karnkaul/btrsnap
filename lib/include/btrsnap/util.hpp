#pragma once
#include "btrsnap/btrfs.hpp"
#include "btrsnap/snapshot.hpp"
#include <optional>
#include <vector>

namespace btrsnap::util {
[[nodiscard]] auto to_snapshot(IBtrfs const& btrfs, fs::path path) -> std::optional<Snapshot>;
[[nodiscard]] auto to_sorted_snapshots(IBtrfs const& btrfs, fs::path const& parent) -> std::vector<Snapshot>;
} // namespace btrsnap::util
