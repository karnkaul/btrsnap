#pragma once
#include "btrsnap/config.hpp"
#include "btrsnap/subvolume.hpp"
#include <iosfwd>

namespace btrsnap {
class Instance {
  public:
	explicit Instance(gsl::not_null<IBtrfs const*> btrfs = &IBtrfs::get_default()) : m_btrfs(btrfs) {}

	auto load_subvolume(Config config) -> Result<void>;
	[[nodiscard]] auto get_loaded_subvolumes() const -> std::span<Subvolume const> { return m_subvolumes; }
	void clear_loaded_subvolumes();

	void print_snapshots(std::ostream& out) const;
	auto take_snapshots(Timestamp timestamp = current_timestamp()) -> std::vector<Result<Snapshot>>;
	auto recycle_snapshots() -> RecycleReport;
	auto clear_all_snapshots() -> std::vector<Result<Snapshot>>;

  private:
	gsl::not_null<IBtrfs const*> m_btrfs;

	std::vector<Subvolume> m_subvolumes{};
};
} // namespace btrsnap
