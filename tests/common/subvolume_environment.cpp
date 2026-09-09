#include "common/subvolume_environment.hpp"
#include <stdexcept>

namespace btrsnap::test {
SubvolumeEnvironment::SubvolumeEnvironment(fs::path const& subpath) : m_subvolume_path(path_to(subpath)) {
	auto const result = MockBtrfs::create_directory(m_subvolume_path.generic_string());
	if (!result) { throw std::runtime_error{result.error().message}; }
}
} // namespace btrsnap::test
