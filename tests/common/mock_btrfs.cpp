#include "common/mock_btrfs.hpp"
#include "detail/to_error.hpp"
#include <format>

namespace btrsnap::test {
auto MockBtrfs::is_directory(fs::path const& path) -> Result<void> {
	if (fs::is_directory(path)) { return {}; }
	return detail::to_error(Error::Type::Btrfs, std::format("Not a directory: {}", path.generic_string()));
}

auto MockBtrfs::create_directory(fs::path const& path) -> Result<void> {
	if (fs::exists(path)) { return detail::to_error(Error::Type::Btrfs, std::format("Already exists: {}", path.generic_string())); }
	if (!fs::create_directories(path)) { return detail::to_error(Error::Type::Btrfs, std::format("Failed to create directory: {}", path.generic_string())); }
	return {};
}

auto MockBtrfs::delete_directory(fs::path const& path) -> Result<void> {
	if (!fs::is_directory(path)) { return detail::to_error(Error::Type::Btrfs, std::format("Not a directory: {}", path.generic_string())); }
	if (!fs::remove_all(path)) { return detail::to_error(Error::Type::Btrfs, std::format("Failed to delete directory: {}", path.generic_string())); }
	return {};
}
} // namespace btrsnap::test
