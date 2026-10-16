#pragma once
#include "btrsnap/config.hpp"
#include "btrsnap/subvolume.hpp"

namespace btrsnap {
class Instance {
  public:
	[[nodiscard]] static auto create(StorageConfig storage_config, gsl::not_null<IBtrfs const*> btrfs = &IBtrfs::get_default()) -> Result<Instance>;

	[[nodiscard]] auto get_btrfs() const -> IBtrfs const& { return *m_btrfs; }
	[[nodiscard]] auto get_storage_config() const -> StorageConfig const& { return m_storage_config; }

	[[nodiscard]] auto load_subvolume(SubvolumeConfig subvolume_config) const -> Result<Subvolume>;
	[[nodiscard]] auto load_subvolume_list(std::vector<SubvolumeConfig> subvolume_config) const -> Subvolume::List;

  private:
	explicit Instance(gsl::not_null<IBtrfs const*> btrfs, StorageConfig storage_config);

	gsl::not_null<IBtrfs const*> m_btrfs;
	StorageConfig m_storage_config;
};
} // namespace btrsnap
