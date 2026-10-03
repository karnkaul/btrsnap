#include "btrsnap/storage.hpp"
#include "detail/to_error.hpp"

namespace btrsnap {
auto Storage::create(StorageInfo const& storage_info, std::string_view const subvolume_name) -> Result<Storage> {
	if (subvolume_name.empty()) { return detail::to_error(Error::Type::InvalidArgument, "Subvolume name is empty"); }
	return Storage{storage_info.get_snapshots_path() / subvolume_name, storage_info.get_archive_path() / subvolume_name};
}

Storage::Storage(fs::path snapshots_directory, fs::path archive_directory)
	: m_snapshots_directory(std::move(snapshots_directory)), m_archive_directory(std::move(archive_directory)) {}
} // namespace btrsnap
