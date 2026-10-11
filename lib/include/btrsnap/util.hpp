#pragma once
#include "btrsnap/btrfs.hpp"
#include "btrsnap/config.hpp"
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

void from_json(dj::Json const& json, StorageConfig& out);
void from_json(dj::Json const& json, RecycleConfig& out);
void from_json(dj::Json const& json, SubvolumeConfig& out);
void from_json(dj::Json const& json, InstanceConfig& out);
} // namespace btrsnap::util
