#pragma once
#include "btrsnap/btrfs.hpp"
#include "btrsnap/config.hpp"
#include "btrsnap/recycle_report.hpp"
#include "btrsnap/result.hpp"
#include "btrsnap/snapshot.hpp"
#include <gsl/pointers>
#include <iosfwd>

namespace btrsnap {
class Subvolume {
  public:
	[[nodiscard]] static auto create(gsl::not_null<IBtrfs const*> btrfs, Config config) -> Result<Subvolume>;

	[[nodiscard]] auto get_config() const -> Config const& { return m_config; }

	[[nodiscard]] auto get_live_snapshots() const -> std::vector<Snapshot>;
	[[nodiscard]] auto get_archived_snapshots() const -> std::vector<Snapshot>;

	[[nodiscard]] auto take_snapshot(Timestamp timestamp) -> Result<Snapshot>;
	auto recycle_snapshots() -> RecycleReport;
	auto clear_all_snapshots() -> std::vector<Result<Snapshot>>;
	void print_snapshots(std::ostream& out, Timestamp now = current_timestamp()) const;

  private:
	explicit Subvolume(gsl::not_null<IBtrfs const*> btrfs, Config config);

	gsl::not_null<IBtrfs const*> m_btrfs;
	Config m_config{};
};
} // namespace btrsnap
