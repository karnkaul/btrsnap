#pragma once
#include "btrsnap/info.hpp"
#include "btrsnap/result.hpp"

namespace btrsnap {
class Storage {
  public:
	[[nodiscard]] static auto create(StorageInfo const& storage_info, std::string_view subvolume_name) -> Result<Storage>;

	[[nodiscard]] auto get_snapshots_directory() const -> fs::path const& { return m_snapshots_directory; }
	[[nodiscard]] auto get_archive_directory() const -> fs::path const& { return m_archive_directory; }

  private:
	explicit Storage(fs::path snapshots_directory, fs::path archive_directory);

	fs::path m_snapshots_directory{};
	fs::path m_archive_directory{};
};
} // namespace btrsnap
