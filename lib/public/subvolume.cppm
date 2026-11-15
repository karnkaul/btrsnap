module;

#include <gsl/pointers>

export module btrsnap.subvolume;

export import btrsnap.util;

import klib.log;

export namespace btrsnap {
class SubvolumeStorage {
  public:
	using Config = StorageConfig;

	[[nodiscard]] static auto create(Config const& config, std::string_view subvolume_name) -> Result<SubvolumeStorage>;

	[[nodiscard]] auto get_snapshots_directory() const -> fs::path const& { return m_snapshots_directory; }
	[[nodiscard]] auto get_archive_directory() const -> fs::path const& { return m_archive_directory; }

  private:
	explicit SubvolumeStorage(fs::path snapshots_directory, fs::path archive_directory);

	fs::path m_snapshots_directory{};
	fs::path m_archive_directory{};
};

class Recycler {
  public:
	using Config = RecycleConfig;
	using Report = RecycleReport;

	explicit Recycler(gsl::not_null<IBtrfs const*> btrfs) : m_btrfs(btrfs) {}

	[[nodiscard]] auto recycle_snapshots(SubvolumeStorage const& storage, Config const& config) const -> Report;

	[[nodiscard]] auto get_sorted_snapshots_in(fs::path const& path) const -> std::vector<Snapshot>;
	[[nodiscard]] static auto get_excess_snapshots(std::vector<Snapshot> sorted, int keep) -> std::vector<Snapshot>;
	[[nodiscard]] auto delete_snapshots(std::vector<Snapshot> snapshots) const -> std::vector<Result<Snapshot>>;

	[[nodiscard]] auto archive_snapshot(Snapshot const& source, fs::path const& parent_directory) const
		-> Result<Snapshot>;
	[[nodiscard]] auto populate_archive(SubvolumeStorage const& storage,
										std::span<Snapshot const> excess,
										std::chrono::days period) const -> std::vector<Snapshot>;

  private:
	klib::log::Typed<Recycler> m_log;

	gsl::not_null<IBtrfs const*> m_btrfs;
};

class Subvolume {
  public:
	using Config = SubvolumeConfig;
	using Storage = SubvolumeStorage;

	[[nodiscard]] static auto
	create(gsl::not_null<IBtrfs const*> btrfs, StorageConfig const& storage_config, Config config) -> Result<Subvolume>;

	[[nodiscard]] auto get_btrfs() const -> IBtrfs const& { return *m_btrfs; }
	[[nodiscard]] auto get_config() const -> Config const& { return m_config; }
	[[nodiscard]] auto get_storage() const -> Storage const& { return m_storage; }

	[[nodiscard]] auto get_live_snapshots() const -> std::vector<Snapshot>;
	[[nodiscard]] auto get_archived_snapshots() const -> std::vector<Snapshot>;

	[[nodiscard]] auto take_snapshot(Timestamp timestamp) -> Result<Snapshot>;
	auto recycle_snapshots() -> RecycleReport;
	auto clear_all_snapshots() -> std::vector<Result<Snapshot>>;
	void print_snapshots(std::ostream& out, Timestamp now = current_timestamp()) const;

  private:
	explicit Subvolume(gsl::not_null<IBtrfs const*> btrfs, Storage storage, Config config);

	[[nodiscard]] auto save_snapshot_to(fs::path const& destination) const -> Result<void>;

	void on_action(Result<Snapshot> const& result, std::string_view action) const;

	klib::log::Typed<Subvolume> m_log;

	gsl::not_null<IBtrfs const*> m_btrfs;
	Storage m_storage;
	Config m_config;
};

class SnapshotList {
  public:
	explicit SnapshotList(Subvolume const& subvolume);

	using Number = SnapshotNumber;

	struct Entry {
		Number number{};
		Snapshot snapshot{};
		bool is_archived{};
	};

	[[nodiscard]] auto get_entries() const -> std::span<Entry const> { return m_entries; }
	[[nodiscard]] auto find_entry(Number number) const -> klib::Ptr<Entry const>;
	[[nodiscard]] auto find_snapshot(Number number) const -> klib::Ptr<Snapshot const>;

	auto delete_snapshot(Number number) -> Result<Snapshot>;

  private:
	gsl::not_null<IBtrfs const*> m_btrfs;
	std::vector<Entry> m_entries{};
};

class SubvolumeList {
  public:
	[[nodiscard]] auto take_snapshots(Timestamp timestamp = current_timestamp()) -> std::vector<Result<Snapshot>>;
	auto recycle_snapshots() -> RecycleReport;
	auto clear_all_snapshots() -> std::vector<Result<Snapshot>>;
	void print_snapshots(std::ostream& out, Timestamp now = current_timestamp()) const;

	[[nodiscard]] auto find_by_name(std::string_view name) const -> klib::Ptr<Subvolume const>;
	[[nodiscard]] auto find_by_path(fs::path const& path) const -> klib::Ptr<Subvolume const>;

	std::vector<Subvolume> subvolumes{};
};
} // namespace btrsnap
