#include "btrsnap/subvolume.hpp"
#include "common/environment.hpp"
#include "klib/unit_test/unit_test.hpp"
#include <unordered_set>

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
	EXPECT(subvolume->get_snapshots_directory() == snapshots_path);
}

TEST_CASE(subvolume_create_without_existing_snapshots_dir) {
	auto const environment = Environment{};

	auto subvolume = environment.create_subvolume(subvolume_subpath_v);
	ASSERT(subvolume.has_value());
	EXPECT(subvolume->get_snapshots_directory() == environment.get_snapshots_path(subvolume_subpath_v));
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

	auto const snapshots = subvolume->get_all_snapshots();
	ASSERT(snapshots.size() == 1);
	auto const& snapshot = snapshots.front();
	EXPECT(snapshot.path.filename().string() == snapshot_name);
}

TEST_CASE(subvolume_trim_snapshots) {
	auto const environment = Environment{};
	auto subvolume = environment.create_subvolume(subvolume_subpath_v);
	ASSERT(subvolume.has_value());

	static constexpr auto create_v{3};
	static constexpr auto keep_v{1};

	auto timestamp = current_timestamp();
	auto expected_trimmed = std::vector<fs::path>{};
	for (auto i = 1; i <= create_v; ++i) {
		auto result = subvolume->take_snapshot(timestamp);
		ASSERT(result.has_value());
		timestamp -= 10min;
		if (i <= keep_v) { continue; }
		expected_trimmed.push_back(std::move(result->path));
	}

	auto const results = subvolume->delete_snapshots(keep_v);
	EXPECT(results.size() == 2);
	auto trimmed = std::unordered_set<fs::path>{};
	for (auto const& result : results) {
		ASSERT(result.has_value());
		trimmed.insert(result->path);
	}

	for (auto const& expected : expected_trimmed) { EXPECT(trimmed.contains(expected)); }
}
} // namespace
} // namespace btrsnap::test
