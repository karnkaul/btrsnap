#pragma once
#include "btrsnap/btrfs.hpp"
#include "btrsnap/info.hpp"
#include "btrsnap/snapshot.hpp"
#include <optional>
#include <vector>

namespace dj {
class Json;
} // namespace dj

namespace btrsnap::util {
[[nodiscard]] auto to_snapshot(IBtrfs const& btrfs, fs::path path) -> std::optional<Snapshot>;
[[nodiscard]] auto list_snapshots(IBtrfs const& btrfs, fs::path const& parent) -> std::vector<Snapshot>;

[[nodiscard]] auto copy_snapshot(IBtrfs const& btrfs, Snapshot const& source, fs::path const& dst_dir) -> Result<Snapshot>;

[[nodiscard]] auto is_non_empty(fs::path const& path) -> Result<void>;
[[nodiscard]] auto is_directory(fs::path const& path) -> Result<void>;
[[nodiscard]] auto ensure_directory(fs::path const& path) -> Result<void>;

void from_json(dj::Json const& json, StorageInfo& out);
void from_json(dj::Json const& json, RecycleInfo& out);
void from_json(dj::Json const& json, SubvolumeInfo& out);
void from_json(dj::Json const& json, InstanceInfo& out);
} // namespace btrsnap::util
