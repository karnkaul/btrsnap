#include "btrsnap/recycler.hpp"
#include "common/environment.hpp"
#include "klib/unit_test/unit_test.hpp"

namespace btrsnap::test {
namespace {
using namespace std::chrono_literals;

struct Fixture {
	static constexpr std::string_view subvolume_subpath_v{"subvol"};
	static constexpr std::string_view subvolume_name_v{"@subvol"};
	static constexpr auto recycle_v = RecycleInfo{.snapshot_limit = 2, .archive_limit = 2, .archive_period = std::chrono::days{1}};

	Environment environment{};
	Recycler recycler{&environment.get_btrfs()};
	Subvolume subvolume{environment.create_subvolume(subvolume_subpath_v, subvolume_name_v, recycle_v).value()};
};

struct Snapshotter {
	void take_snapshots(int count) {
		for (; count > 0; --count) {
			auto result = subvolume.take_snapshot(next_timestamp);
			EXPECT(result);
			next_timestamp += delta;
		}
	}

	Subvolume& subvolume;

	Seconds delta{12h};
	Seconds next_timestamp{current_timestamp()};
};

TEST_CASE(recycler_empty_subvolume) {
	auto fixture = Fixture{};

	auto snapshots = fixture.recycler.get_sorted_snapshots_in(fixture.subvolume.get_storage().get_snapshots_directory());
	EXPECT(snapshots.empty());
	snapshots = fixture.recycler.get_sorted_snapshots_in(fixture.subvolume.get_storage().get_archive_directory());
	EXPECT(snapshots.empty());
}

TEST_CASE(recycler_noop) {
	auto fixture = Fixture{};
	Snapshotter{.subvolume = fixture.subvolume, .delta = 24h}.take_snapshots(Fixture::recycle_v.snapshot_limit);
	auto live_snapshots = fixture.recycler.get_sorted_snapshots_in(fixture.subvolume.get_storage().get_snapshots_directory());
	auto excess_snapshots = Recycler::get_excess_snapshots(std::move(live_snapshots), Fixture::recycle_v.snapshot_limit);
	EXPECT(excess_snapshots.empty());
}

TEST_CASE(recycler_archive_single) {
	auto fixture = Fixture{};

	auto const expected_timestamp = current_timestamp();
	Snapshotter{.subvolume = fixture.subvolume, .delta = 12h, .next_timestamp = expected_timestamp}.take_snapshots(Fixture::recycle_v.snapshot_limit + 1);

	auto live_snapshots = fixture.recycler.get_sorted_snapshots_in(fixture.subvolume.get_storage().get_snapshots_directory());
	auto snapshots = Recycler::get_excess_snapshots(std::move(live_snapshots), Fixture::recycle_v.snapshot_limit);

	ASSERT(snapshots.size() == 1);
	EXPECT(snapshots.front().timestamp == expected_timestamp);

	snapshots = fixture.recycler.populate_archive(fixture.subvolume.get_storage(), std::move(snapshots), Fixture::recycle_v.archive_period);
	ASSERT(snapshots.size() == 1);
	EXPECT(snapshots.front().timestamp == expected_timestamp);

	snapshots = fixture.recycler.get_sorted_snapshots_in(fixture.subvolume.get_storage().get_archive_directory());
	ASSERT(snapshots.size() == 1);
	EXPECT(snapshots.front().timestamp == expected_timestamp);

	snapshots = fixture.recycler.get_sorted_snapshots_in(fixture.subvolume.get_storage().get_snapshots_directory());
	EXPECT(snapshots.size() == 2);
	for (auto const& snapshot : snapshots) { EXPECT(snapshot.timestamp > expected_timestamp); }
}

TEST_CASE(recycler_recycle_multiple) {
	auto fixture = Fixture{};

	auto snapshotter = Snapshotter{.subvolume = fixture.subvolume, .delta = 12h};

	auto report = fixture.recycler.recycle_snapshots(fixture.subvolume.get_storage(), Fixture::recycle_v);
	EXPECT(report.archived.empty());
	EXPECT(report.deleted.empty());

	snapshotter.take_snapshots(Fixture::recycle_v.snapshot_limit + 1);								  // limit + 1 live
	report = fixture.recycler.recycle_snapshots(fixture.subvolume.get_storage(), Fixture::recycle_v); // limit live
	EXPECT(report.archived.size() == 1);
	EXPECT(report.deleted.empty());

	snapshotter.take_snapshots(3); // limit + 3 live, 1 archive
	auto const pre_recycle_count = static_cast<int>(fixture.subvolume.get_live_snapshots().size() + fixture.subvolume.get_archived_snapshots().size());
	EXPECT(pre_recycle_count == Fixture::recycle_v.snapshot_limit + 3 + 1);

	report = fixture.recycler.recycle_snapshots(fixture.subvolume.get_storage(), Fixture::recycle_v); // limit live, limit archive
	auto const post_recycle_count = static_cast<int>(fixture.subvolume.get_live_snapshots().size() + fixture.subvolume.get_archived_snapshots().size());
	EXPECT(post_recycle_count == Fixture::recycle_v.snapshot_limit + Fixture::recycle_v.archive_limit);

	EXPECT(int(report.deleted.size()) == pre_recycle_count - post_recycle_count);
}
} // namespace
} // namespace btrsnap::test
