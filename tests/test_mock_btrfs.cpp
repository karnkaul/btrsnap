#include "common/mock_btrfs.hpp"
#include "common/test_directory.hpp"
#include "klib/unit_test/unit_test.hpp"

namespace btrsnap::test {
namespace {
TEST_CASE(mock_btrfs) {
	auto const test_dir = TestDirectory{};
	auto const btrfs = MockBtrfs{};

	auto const subvolume = (test_dir / "subvol").string();
	auto result = btrfs.is_subvolume(subvolume);
	EXPECT(!result);
	result = btrfs.create_subvolume(subvolume);
	EXPECT(result);
	result = btrfs.is_subvolume(subvolume);
	EXPECT(result);
	result = btrfs.create_subvolume(subvolume);
	EXPECT(!result);
}
} // namespace
} // namespace btrsnap::test
