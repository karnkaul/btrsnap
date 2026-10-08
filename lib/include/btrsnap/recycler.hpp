#pragma once
#include "btrsnap/btrfs.hpp"
#include "btrsnap/info.hpp"
#include "btrsnap/recycle_report.hpp"
#include "btrsnap/storage.hpp"
#include <gsl/pointers>

namespace btrsnap {
class Recycler {
  public:
	using Info = RecycleInfo;
	using Report = RecycleReport;

	explicit Recycler(gsl::not_null<IBtrfs const*> btrfs) : m_btrfs(btrfs) {}

	[[nodiscard]] auto recycle_snapshots(Storage const& storage, Info const& info) const -> Report;

	[[nodiscard]] auto get_snapshot_list(Storage const& storage) const -> SnapshotList;

	[[nodiscard]] auto get_sorted_snapshots_in(fs::path const& path) const -> std::vector<Snapshot>;
	[[nodiscard]] static auto get_excess_snapshots(std::vector<Snapshot> sorted, int keep) -> std::vector<Snapshot>;
	[[nodiscard]] auto delete_snapshots(std::vector<Snapshot> snapshots) const -> std::vector<Result<Snapshot>>;

	[[nodiscard]] auto archive_snapshot(Snapshot const& source, fs::path const& parent_directory) const -> Result<Snapshot>;
	[[nodiscard]] auto populate_archive(Storage const& storage, std::span<Snapshot const> excess, std::chrono::days period) const -> std::vector<Snapshot>;

  private:
	gsl::not_null<IBtrfs const*> m_btrfs;
};
} // namespace btrsnap
