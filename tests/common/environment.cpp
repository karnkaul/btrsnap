#include "common/environment.hpp"
#include <stdexcept>

namespace btrsnap::test {
auto Environment::get_snapshots_path(std::string_view subvolume_subpath) const -> fs::path { return path_to(subvolume_subpath) / snapshots_subdirectory_v; }

auto Environment::create_subvolume(std::string_view const subpath) const -> Result<Subvolume> {
	auto path = path_to(subpath);
	if (!fs::is_directory(path)) {
		auto const result = MockBtrfs::create_directory(path.generic_string());
		if (!result) { throw std::runtime_error{result.error().message}; }
	}

	auto const config = Config{
		.subvolume = path.generic_string(),
		.snapshots_subdirectory = std::string{snapshots_subdirectory_v},
		.archive_subdirectory = std::string{archive_subdirectory_v},
	};

	return Subvolume::create(&get_btrfs(), config);
}
} // namespace btrsnap::test
