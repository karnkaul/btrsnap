#include "btrsnap/subvolume.hpp"
#include "common/subvolume_environment.hpp"
#include "klib/unit_test/unit_test.hpp"

namespace btrsnap::test {
namespace {
TEST_CASE(subvolume_create_with_existing_snapshots_dir) {
	auto const environment = SubvolumeEnvironment{};

	ASSERT(fs::create_directories(environment.get_snapshots_path()));

	auto subvolume = environment.create_subvolume();
	ASSERT(subvolume.has_value());
	EXPECT(subvolume->get_path() == environment.get_subvolume_path());
	EXPECT(subvolume->get_snapshot_directory() == environment.get_snapshots_path());
}

TEST_CASE(subvolume_create_without_existing_snapshots_dir) {
	auto const environment = SubvolumeEnvironment{};

	auto subvolume = environment.create_subvolume();
	ASSERT(subvolume.has_value());
	EXPECT(subvolume->get_path() == environment.get_subvolume_path());
	EXPECT(subvolume->get_snapshot_directory() == environment.get_snapshots_path());
}

TEST_CASE(subvolume_take_snapshot) {
	auto const environment = SubvolumeEnvironment{};
	auto subvolume = environment.create_subvolume();
	ASSERT(subvolume.has_value());

	auto const zoned_seconds = to_zoned_seconds(Clock::now());
	auto result = subvolume->take_snapshot(zoned_seconds);
	ASSERT(result.has_value());

	auto const snapshot_name = to_pathname(zoned_seconds);
	EXPECT(result->path.filename().string() == snapshot_name);

	auto const snapshots = subvolume->get_all_snapshots();
	ASSERT(snapshots.size() == 1);
	auto const& snapshot = snapshots.front();
	EXPECT(snapshot.path.filename().string() == snapshot_name);
}
} // namespace
} // namespace btrsnap::test
