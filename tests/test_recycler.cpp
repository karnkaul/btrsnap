#include "btrsnap/recycler.hpp"
#include "common/environment.hpp"
#include "klib/unit_test/unit_test.hpp"

namespace btrsnap::test {
namespace {
using namespace std::chrono_literals;

struct Fixture {
	static constexpr std::string_view subvolume_subpath_v{"subvol"};

	explicit Fixture() { fs::create_directories(config.get_subvolume_path()); }

	Environment environment{};
	Recycler recycler{&environment.get_btrfs()};
	Config config{environment.create_config(subvolume_subpath_v)};
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

	auto snapshots = fixture.recycler.get_live_snapshots(fixture.config);
	EXPECT(snapshots.empty());
	snapshots = fixture.recycler.get_archived_snapshots(fixture.config);
	EXPECT(snapshots.empty());
	auto results = fixture.recycler.delete_excess_live_snapshots(fixture.config);
	EXPECT(results.empty());
	results = fixture.recycler.delete_excess_archived_snapshots(fixture.config);
	EXPECT(snapshots.empty());
}

TEST_CASE(recycler_noop) {
	auto fixture = Fixture{};

	fixture.config.snapshot_limit = 2;

	auto subvolume = fixture.environment.create_subvolume(Fixture::subvolume_subpath_v);
	ASSERT(subvolume.has_value());
	Snapshotter{.subvolume = *subvolume, .delta = 24h}.take_snapshots(2);

	auto snapshots = fixture.recycler.delete_excess_live_snapshots(fixture.config);
	EXPECT(snapshots.empty());
}

TEST_CASE(recycler_archive_single) {
	auto fixture = Fixture{};

	fixture.config.snapshot_limit = 2;

	auto subvolume = fixture.environment.create_subvolume(Fixture::subvolume_subpath_v);
	ASSERT(subvolume.has_value());
	auto const expected_timestamp = current_timestamp();
	Snapshotter{.subvolume = *subvolume, .delta = 12h, .next_timestamp = expected_timestamp}.take_snapshots(3);

	auto snapshots = fixture.recycler.get_excess_live_snapshots(fixture.config);
	EXPECT(snapshots.size() == 1);
	EXPECT(snapshots.front().timestamp == expected_timestamp);
	snapshots = fixture.recycler.populate_archive(fixture.config, std::move(snapshots));
	EXPECT(snapshots.size() == 1);
	EXPECT(snapshots.front().timestamp == expected_timestamp);

	snapshots = fixture.recycler.get_archived_snapshots(fixture.config);
	EXPECT(snapshots.size() == 1);
	EXPECT(snapshots.front().timestamp == expected_timestamp);

	snapshots = fixture.recycler.get_live_snapshots(fixture.config);
	EXPECT(snapshots.size() == 2);
	for (auto const& snapshot : snapshots) { EXPECT(snapshot.timestamp > expected_timestamp); }
}

TEST_CASE(recycler_archive_multiple) {
	auto fixture = Fixture{};

	fixture.config.snapshot_limit = 2;
	fixture.config.archive_limit = 2;
	fixture.config.archive_period = std::chrono::days{1};

	auto subvolume = fixture.environment.create_subvolume(Fixture::subvolume_subpath_v);
	ASSERT(subvolume.has_value());
	Snapshotter{.subvolume = *subvolume, .delta = 6h}.take_snapshots(10);

	auto snapshots = fixture.recycler.get_excess_live_snapshots(fixture.config);
	snapshots = fixture.recycler.populate_archive(fixture.config, std::move(snapshots));

	ASSERT(snapshots.size() == 2);
	auto const delta_time = Seconds{std::abs((snapshots[0].timestamp - snapshots[1].timestamp).count())};
	EXPECT(delta_time >= fixture.config.archive_period);
}

TEST_CASE(recycler_recycle_multiple) {
	auto fixture = Fixture{};

	fixture.config.snapshot_limit = 3;
	fixture.config.archive_limit = 3;
	fixture.config.archive_period = std::chrono::days{1};

	auto subvolume = fixture.environment.create_subvolume(Fixture::subvolume_subpath_v);
	ASSERT(subvolume.has_value());
	auto snapshotter = Snapshotter{.subvolume = *subvolume, .delta = 12h};

	auto report = fixture.recycler.recycle_snapshots(fixture.config);
	EXPECT(report.archived.empty());
	EXPECT(report.deleted.empty());

	snapshotter.take_snapshots(4); // 3 + 1 live
	report = fixture.recycler.recycle_snapshots(fixture.config);
	EXPECT(report.archived.size() == 1);
	EXPECT(report.deleted.empty());

	snapshotter.take_snapshots(4); // 3 + 4 live
	report = fixture.recycler.recycle_snapshots(fixture.config);
	EXPECT(report.archived.size() == 2);
	EXPECT(report.deleted.size() == 2);
}
} // namespace
} // namespace btrsnap::test
