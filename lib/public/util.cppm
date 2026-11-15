export module btrsnap.util;

export import djson;
export import btrsnap.btrfs;
export import btrsnap.config;
export import btrsnap.types;

export namespace btrsnap::util {
[[nodiscard]] auto to_snapshot(IBtrfs const& btrfs, fs::path path) -> std::optional<Snapshot>;
[[nodiscard]] auto list_snapshots(IBtrfs const& btrfs, fs::path const& parent) -> std::vector<Snapshot>;

[[nodiscard]] auto copy_snapshot(IBtrfs const& btrfs, Snapshot const& source, fs::path const& dst_dir)
	-> Result<Snapshot>;

[[nodiscard]] auto is_non_empty(fs::path const& path) -> Result<void>;
[[nodiscard]] auto is_directory(fs::path const& path) -> Result<void>;
[[nodiscard]] auto ensure_directory(fs::path const& path) -> Result<void>;

void from_json(dj::Json const& json, StorageConfig& out);
void from_json(dj::Json const& json, RecycleConfig& out);
void from_json(dj::Json const& json, SubvolumeConfig& out);
void from_json(dj::Json const& json, InstanceConfig& out);

[[nodiscard]] auto load_config(std::string_view path) -> Result<InstanceConfig>;
} // namespace btrsnap::util
