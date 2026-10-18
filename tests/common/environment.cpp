#include "common/environment.hpp"
#include <stdexcept>

namespace btrsnap::test {
auto Environment::create_subvolume_config(std::string_view const subvolume_subpath, std::string_view const name, RecycleConfig const recycle) const
	-> SubvolumeConfig {
	return SubvolumeConfig{
		.name = std::string{name},
		.path = path_to(subvolume_subpath),
		.recycle = recycle,
	};
}

auto Environment::create_subvolume(std::string_view const subpath, std::string_view const name, RecycleConfig const recycle) const -> Result<Subvolume> {
	auto path = path_to(subpath);
	if (!fs::is_directory(path)) {
		auto const result = MockBtrfs::create_directory(path.generic_string());
		if (!result) { throw std::runtime_error{result.error().message}; }
	}

	auto subvolume_config = create_subvolume_config(subpath, name, recycle);
	return Subvolume::create(&get_btrfs(), get_storage_config(), std::move(subvolume_config));
}
} // namespace btrsnap::test
