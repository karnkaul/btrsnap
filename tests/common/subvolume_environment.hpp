#pragma once
#include "btrsnap/subvolume.hpp"
#include "common/mock_environment.hpp"

namespace btrsnap::test {
class SubvolumeEnvironment : public MockEnvironment {
  public:
	static constexpr std::string_view snapshots_subdirectory_v{".snapshots"};

	explicit SubvolumeEnvironment(fs::path const& subpath = "subvol");

	[[nodiscard]] auto get_subvolume_path() const -> fs::path const& { return m_subvolume_path; }
	[[nodiscard]] auto get_snapshots_path() const -> fs::path { return get_subvolume_path() / snapshots_subdirectory_v; }

	[[nodiscard]] auto create_subvolume() const -> Result<Subvolume> { return Subvolume::create(&get_btrfs(), get_subvolume_path(), snapshots_subdirectory_v); }

  private:
	fs::path m_subvolume_path{};
};
} // namespace btrsnap::test
