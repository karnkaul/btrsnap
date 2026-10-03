#pragma once
#include "btrsnap/btrfs.hpp"
#include "btrsnap/info.hpp"
#include "btrsnap/recycle_report.hpp"
#include "btrsnap/result.hpp"
#include "btrsnap/snapshot.hpp"
#include "btrsnap/storage.hpp"
#include <gsl/pointers>
#include <iosfwd>

namespace btrsnap {
class Subvolume {
  public:
	using Info = SubvolumeInfo;

	[[nodiscard]] static auto create(gsl::not_null<IBtrfs const*> btrfs, StorageInfo const& storage_info, Info info) -> Result<Subvolume>;

	[[nodiscard]] auto get_info() const -> Info const& { return m_info; }
	[[nodiscard]] auto get_storage() const -> Storage const& { return m_storage; }

	[[nodiscard]] auto get_live_snapshots() const -> std::vector<Snapshot>;
	[[nodiscard]] auto get_archived_snapshots() const -> std::vector<Snapshot>;

	[[nodiscard]] auto take_snapshot(Timestamp timestamp) -> Result<Snapshot>;

	auto recycle_snapshots() -> RecycleReport;
	auto clear_all_snapshots() -> std::vector<Result<Snapshot>>;
	void print_snapshots(std::ostream& out, Timestamp now = current_timestamp()) const;

  private:
	explicit Subvolume(gsl::not_null<IBtrfs const*> btrfs, Storage storage, Info info);

	gsl::not_null<IBtrfs const*> m_btrfs;
	Storage m_storage;
	Info m_info;
};
} // namespace btrsnap
