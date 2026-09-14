#pragma once
#include "btrsnap/config.hpp"
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

	[[nodiscard]] auto path_to(std::string_view const subpath) const -> fs::path { return get_test_directory() / subpath; }

	[[nodiscard]] auto create_config(std::string_view const subvolume_subpath) const -> Config {
		return Config{.subvolume = (path_to(subvolume_subpath)).string()};
	}

	[[nodiscard]] auto get_snapshots_path(std::string_view subvolume_subpath) const -> fs::path;

	[[nodiscard]] auto create_subvolume(std::string_view subpath) const -> Result<Subvolume>;

  private:
	TestDirectory m_test_dir{};
	MockBtrfs m_btrfs{};
};
} // namespace btrsnap::test
