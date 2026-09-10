#include "common/environment.hpp"
#include <stdexcept>

namespace btrsnap::test {
auto Environment::get_snapshots_path(std::string_view subvolume_subpath) const -> fs::path { return path_to(subvolume_subpath) / snapshots_subdirectory_v; }

auto Environment::create_subvolume(std::string_view const subpath) const -> Result<Subvolume> {
	auto const path = path_to(subpath);
	if (!fs::is_directory(path)) {
		auto const result = MockBtrfs::create_directory(path.generic_string());
		if (!result) { throw std::runtime_error{result.error().message}; }
	}
	return Subvolume::create(&get_btrfs(), path, snapshots_subdirectory_v);
}
} // namespace btrsnap::test
