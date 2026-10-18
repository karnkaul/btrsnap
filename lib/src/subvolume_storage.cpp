#include "btrsnap/subvolume_storage.hpp"
#include "detail/to_error.hpp"

namespace btrsnap {
auto SubvolumeStorage::create(StorageConfig const& config, std::string_view const subvolume_name) -> Result<SubvolumeStorage> {
	if (subvolume_name.empty()) { return detail::to_error(Error::Type::InvalidArgument, "Subvolume name is empty"); }
	return SubvolumeStorage{config.get_snapshots_path() / subvolume_name, config.get_archive_path() / subvolume_name};
}

SubvolumeStorage::SubvolumeStorage(fs::path snapshots_directory, fs::path archive_directory)
	: m_snapshots_directory(std::move(snapshots_directory)), m_archive_directory(std::move(archive_directory)) {}
} // namespace btrsnap
