#pragma once
#include "btrsnap/config.hpp"
#include "btrsnap/subvolume.hpp"
#include <iosfwd>

namespace btrsnap {
class Instance {
  public:
	[[nodiscard]] static auto create(StorageConfig storage_config, gsl::not_null<IBtrfs const*> btrfs = &IBtrfs::get_default()) -> Result<Instance>;

	[[nodiscard]] auto get_btrfs() const -> IBtrfs const& { return *m_btrfs; }
	[[nodiscard]] auto get_storage_config() const -> StorageConfig const& { return m_storage_config; }

	auto load_subvolume(SubvolumeConfig subvolume_config) -> Result<void>;
	void load_subvolumes(std::vector<SubvolumeConfig> subvolume_configs);
	[[nodiscard]] auto get_loaded_subvolumes() const -> std::span<Subvolume const> { return m_subvolumes; }
	void clear_loaded_subvolumes();

	auto take_snapshots(Timestamp timestamp = current_timestamp()) -> std::vector<Result<Snapshot>>;
	auto recycle_snapshots() -> RecycleReport;
	auto clear_all_snapshots() -> std::vector<Result<Snapshot>>;
	void print_snapshots(std::ostream& out) const;

  private:
	explicit Instance(gsl::not_null<IBtrfs const*> btrfs, StorageConfig storage_config);

	gsl::not_null<IBtrfs const*> m_btrfs;
	StorageConfig m_storage_config;

	std::vector<Subvolume> m_subvolumes{};
};
} // namespace btrsnap
