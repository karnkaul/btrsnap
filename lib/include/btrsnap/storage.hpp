#pragma once
#include "btrsnap/config.hpp"
#include "btrsnap/result.hpp"

namespace btrsnap {
class Storage {
  public:
	using Config = StorageConfig;

	[[nodiscard]] static auto create(Config const& config, std::string_view subvolume_name) -> Result<Storage>;

	[[nodiscard]] auto get_snapshots_directory() const -> fs::path const& { return m_snapshots_directory; }
	[[nodiscard]] auto get_archive_directory() const -> fs::path const& { return m_archive_directory; }

  private:
	explicit Storage(fs::path snapshots_directory, fs::path archive_directory);

	fs::path m_snapshots_directory{};
	fs::path m_archive_directory{};
};
} // namespace btrsnap
