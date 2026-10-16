#include "common/mock_btrfs.hpp"
#include "common/test_directory.hpp"
#include "klib/file_io.hpp"
#include "klib/unit_test/unit_test.hpp"

namespace btrsnap::test {
namespace {
TEST_CASE(mock_btrfs_subvolumes) {
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

TEST_CASE(mock_btrfs_snapshot) {
	auto const test_dir = TestDirectory{};
	auto const btrfs = MockBtrfs{};
	auto const src = (test_dir / "src").string();
	auto const dst = (test_dir / "dst").string();

	EXPECT(MockBtrfs::create_directory(src));
	EXPECT(klib::write_to_file("test", (fs::path{src} / "test.txt").generic_string()));
	auto result = btrfs.create_snapshot(src, dst);
	EXPECT(result);
	auto dst_file = fs::path{dst} / "test.txt";
	EXPECT(fs::is_regular_file(dst_file));
	auto text = std::string{};
	EXPECT(klib::read_file_bytes_to(text, dst_file.generic_string()));
	EXPECT(text == "test");
}
} // namespace
} // namespace btrsnap::test
