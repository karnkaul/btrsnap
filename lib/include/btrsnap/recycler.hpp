#pragma once
#include "btrsnap/btrfs.hpp"
#include "btrsnap/config.hpp"
#include "btrsnap/recycle_report.hpp"
#include "klib/log/typed.hpp"
#include <gsl/pointers>

namespace btrsnap {
class Recycler {
  public:
	using Report = RecycleReport;

	explicit Recycler(gsl::not_null<IBtrfs const*> btrfs) : m_btrfs(btrfs) {}

	[[nodiscard]] auto recycle_snapshots(Config const& config) const -> Report;

	[[nodiscard]] auto get_live_snapshots(Config const& config) const -> std::vector<Snapshot>;
	[[nodiscard]] auto get_archived_snapshots(Config const& config) const -> std::vector<Snapshot>;

	[[nodiscard]] auto get_excess_live_snapshots(Config const& config) const -> std::vector<Snapshot>;
	[[nodiscard]] auto get_excess_archived_snapshots(Config const& config) const -> std::vector<Snapshot>;

	[[nodiscard]] auto delete_snapshots(std::vector<Snapshot> snapshots) const -> std::vector<Result<Snapshot>>;
	[[nodiscard]] auto delete_excess_live_snapshots(Config const& config) const -> std::vector<Result<Snapshot>>;
	[[nodiscard]] auto delete_excess_archived_snapshots(Config const& config) const -> std::vector<Result<Snapshot>>;

	[[nodiscard]] auto populate_archive(Config const& config, std::span<Snapshot const> excess) const -> std::vector<Snapshot>;

  private:
	[[nodiscard]] auto get_sorted_snapshots_in(fs::path const& path) const -> std::vector<Snapshot>;

	klib::log::Typed<Recycler> m_log{};

	gsl::not_null<IBtrfs const*> m_btrfs;
};
} // namespace btrsnap
