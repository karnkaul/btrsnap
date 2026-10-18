#pragma once
#include "btrsnap/subvolume.hpp"
#include "common/mock_btrfs.hpp"
#include "common/test_directory.hpp"

namespace btrsnap::test {
class Environment {
  public:
	static constexpr std::string_view snapshots_subdirectory_v{".snapshots"};
	static constexpr std::string_view archive_subdirectory_v{".archive"};

	[[nodiscard]] auto get_btrfs() const -> IBtrfs const& { return m_btrfs; }
	[[nodiscard]] auto get_test_directory() const -> fs::path const& { return m_test_dir.get_path(); }
	[[nodiscard]] auto get_storage_config() const -> StorageConfig const& { return m_storage_config; }

	[[nodiscard]] auto path_to(std::string_view const subpath) const -> fs::path { return get_test_directory() / subpath; }

	[[nodiscard]] auto create_subvolume_config(std::string_view subvolume_subpath, std::string_view name = "subvol", RecycleConfig recycle = {}) const
		-> SubvolumeConfig;
	[[nodiscard]] auto create_subvolume(std::string_view subpath, std::string_view name = "subvol", RecycleConfig recycle = {}) const -> Result<Subvolume>;

  private:
	TestDirectory m_test_dir{};
	MockBtrfs m_btrfs{};
	StorageConfig m_storage_config{.root = m_test_dir.get_path() / "storage"};
};
} // namespace btrsnap::test
