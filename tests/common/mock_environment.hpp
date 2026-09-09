#pragma once
#include "btrsnap/config.hpp"
#include "common/mock_btrfs.hpp"
#include "common/test_directory.hpp"

namespace btrsnap::test {
class MockEnvironment {
  public:
	[[nodiscard]] auto get_test_directory() const -> fs::path const& { return m_test_dir.get_path(); }

	[[nodiscard]] auto path_to(fs::path const& subpath) const -> fs::path { return get_test_directory() / subpath; }

	[[nodiscard]] auto create_config(std::string_view const subvolume_subpath) const -> Config {
		return Config{.subvolume = (path_to(subvolume_subpath)).string()};
	}

  private:
	TestDirectory m_test_dir{};
	MockBtrfs m_btrfs{};
};
} // namespace btrsnap::test
