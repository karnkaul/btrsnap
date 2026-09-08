#pragma once
#include "btrsnap/instance.hpp"
#include "common/mock_btrfs.hpp"
#include "common/test_directory.hpp"

namespace btrsnap::test {
class TestInstance : public Instance {
  public:
	explicit TestInstance() : Instance(std::make_unique<MockBtrfs>()) {}

	[[nodiscard]] auto get_test_directory() const -> fs::path const& { return m_test_dir.get_path(); }

	[[nodiscard]] auto create_config(std::string_view subvolume_subpath) const -> Config {
		return Config{.subvolume = (get_test_directory() / subvolume_subpath).string()};
	}

  private:
	TestDirectory m_test_dir{};
};
} // namespace btrsnap::test
