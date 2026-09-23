#include "btrsnap/subvolume.hpp"
#include "common/environment.hpp"
#include "klib/unit_test/unit_test.hpp"

namespace btrsnap::test {
namespace {
using namespace std::chrono_literals;

constexpr std::string_view subvolume_subpath_v{"subvol"};

TEST_CASE(subvolume_create_with_existing_snapshots_dir) {
	auto const environment = Environment{};

	auto const snapshots_path = environment.get_snapshots_path(subvolume_subpath_v);
	ASSERT(environment.get_btrfs().create_subvolume(snapshots_path.generic_string()));

	auto subvolume = environment.create_subvolume(subvolume_subpath_v);
	ASSERT(subvolume.has_value());
	EXPECT(subvolume->get_config().get_snapshots_path() == snapshots_path);
}

TEST_CASE(subvolume_create_without_existing_snapshots_dir) {
	auto const environment = Environment{};

	auto subvolume = environment.create_subvolume(subvolume_subpath_v);
	ASSERT(subvolume.has_value());
	EXPECT(subvolume->get_config().get_snapshots_path() == environment.get_snapshots_path(subvolume_subpath_v));
}

TEST_CASE(subvolume_take_snapshot) {
	auto const environment = Environment{};
	auto subvolume = environment.create_subvolume(subvolume_subpath_v);
	ASSERT(subvolume.has_value());

	auto const timestamp = current_timestamp();
	auto result = subvolume->take_snapshot(timestamp);
	ASSERT(result.has_value());

	auto const snapshot_name = to_pathname(timestamp);
	EXPECT(result->path.filename().string() == snapshot_name);

	auto const snapshots = subvolume->get_live_snapshots();
	ASSERT(snapshots.size() == 1);
	EXPECT(snapshots.front().path.filename().string() == snapshot_name);
}

TEST_CASE(subvolume_clear_snapshots) {
	auto const environment = Environment{};
	auto subvolume = environment.create_subvolume(subvolume_subpath_v);
	ASSERT(subvolume.has_value());

	auto timestamp = current_timestamp();
	for (auto i = 0; i < 5; ++i) {
		auto result = subvolume->take_snapshot(timestamp);
		ASSERT(result.has_value());
		timestamp += 30min;
	}

	auto snapshots = subvolume->clear_all_snapshots();
	EXPECT(snapshots.size() == 5);
}
} // namespace
} // namespace btrsnap::test
