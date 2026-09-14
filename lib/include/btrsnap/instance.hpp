#pragma once
#include "btrsnap/config.hpp"
#include "btrsnap/subvolume.hpp"
#include <iosfwd>

namespace btrsnap {
class Instance {
  public:
	explicit Instance(gsl::not_null<IBtrfs const*> btrfs = &IBtrfs::get_default()) : m_btrfs(btrfs) {}

	auto load_subvolume(Config const& config) -> Result<void>;
	[[nodiscard]] auto get_loaded_subvolumes() const -> std::span<Subvolume const> { return m_subvolumes; }
	void clear_loaded_subvolumes();

	void print_snapshots(std::ostream& out) const;
	auto take_snapshots(Timestamp timestamp = current_timestamp()) -> std::vector<Result<Snapshot>>;
	auto trim_snapshots() -> std::vector<Result<Snapshot>>;
	auto clear_snapshots() -> std::vector<Result<Snapshot>>;

  private:
	auto delete_snapshots(int keep) -> std::vector<Result<Snapshot>>;

	gsl::not_null<IBtrfs const*> m_btrfs;

	std::vector<Subvolume> m_subvolumes{};
};
} // namespace btrsnap
