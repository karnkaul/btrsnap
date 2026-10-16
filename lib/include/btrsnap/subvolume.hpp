#pragma once
#include "btrsnap/btrfs.hpp"
#include "btrsnap/config.hpp"
#include "btrsnap/recycle_report.hpp"
#include "btrsnap/result.hpp"
#include "btrsnap/snapshot.hpp"
#include "btrsnap/subvolume_storage.hpp"
#include <gsl/pointers>
#include <iosfwd>
#include <vector>

namespace btrsnap {
class Subvolume {
  public:
	using Config = SubvolumeConfig;
	using Storage = SubvolumeStorage;

	class List;

	[[nodiscard]] static auto create(gsl::not_null<IBtrfs const*> btrfs, StorageConfig const& storage_config, Config config) -> Result<Subvolume>;

	[[nodiscard]] auto get_btrfs() const -> IBtrfs const& { return *m_btrfs; }
	[[nodiscard]] auto get_config() const -> Config const& { return m_config; }
	[[nodiscard]] auto get_storage() const -> Storage const& { return m_storage; }

	[[nodiscard]] auto get_snapshot_list() const -> Snapshot::List;
	[[nodiscard]] auto get_live_snapshots() const -> std::vector<Snapshot>;
	[[nodiscard]] auto get_archived_snapshots() const -> std::vector<Snapshot>;

	[[nodiscard]] auto take_snapshot(Timestamp timestamp) -> Result<Snapshot>;
	auto recycle_snapshots() -> RecycleReport;
	auto clear_all_snapshots() -> std::vector<Result<Snapshot>>;
	void print_snapshots(std::ostream& out, Timestamp now = current_timestamp()) const;

  private:
	explicit Subvolume(gsl::not_null<IBtrfs const*> btrfs, Storage storage, Config config);

	gsl::not_null<IBtrfs const*> m_btrfs;
	Storage m_storage;
	Config m_config;
};

class Subvolume::List {
  public:
	auto take_snapshots(Timestamp timestamp = current_timestamp()) -> std::vector<Result<Snapshot>>;
	auto recycle_snapshots() -> RecycleReport;
	auto clear_all_snapshots() -> std::vector<Result<Snapshot>>;
	void print_snapshots(std::ostream& out, Timestamp now = current_timestamp()) const;

	std::vector<Subvolume> subvolumes{};
};
} // namespace btrsnap
