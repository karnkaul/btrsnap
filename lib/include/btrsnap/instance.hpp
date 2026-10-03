#pragma once
#include "btrsnap/info.hpp"
#include "btrsnap/subvolume.hpp"
#include <iosfwd>

namespace btrsnap {
class Instance {
  public:
	[[nodiscard]] static auto create(StorageInfo storage_info, gsl::not_null<IBtrfs const*> btrfs = &IBtrfs::get_default()) -> Result<Instance>;

	[[nodiscard]] auto get_btrfs() const -> IBtrfs const& { return *m_btrfs; }
	[[nodiscard]] auto get_storage_info() const -> StorageInfo const& { return m_storage_info; }

	auto load_subvolume(SubvolumeInfo subvolume_info) -> Result<void>;
	void load_subvolumes(std::vector<SubvolumeInfo> subvolume_infos);
	[[nodiscard]] auto get_loaded_subvolumes() const -> std::span<Subvolume const> { return m_subvolumes; }
	void clear_loaded_subvolumes();

	auto take_snapshots(Timestamp timestamp = current_timestamp()) -> std::vector<Result<Snapshot>>;
	auto recycle_snapshots() -> RecycleReport;
	auto clear_all_snapshots() -> std::vector<Result<Snapshot>>;
	void print_snapshots(std::ostream& out) const;

  private:
	explicit Instance(gsl::not_null<IBtrfs const*> btrfs, StorageInfo storage_info);

	gsl::not_null<IBtrfs const*> m_btrfs;
	StorageInfo m_storage_info;

	std::vector<Subvolume> m_subvolumes{};
};
} // namespace btrsnap
