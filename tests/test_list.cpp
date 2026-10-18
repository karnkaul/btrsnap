#include "btrsnap/list.hpp"
#include "common/environment.hpp"
#include "klib/unit_test/unit_test.hpp"
#include <algorithm>

namespace btrsnap::test {
namespace {
constexpr std::string_view subvolume_subpath_v{"subvol"};

TEST_CASE(snapshot_list_delete) {
	auto const environment = Environment{};
	auto subvolume = environment.create_subvolume(subvolume_subpath_v);
	ASSERT(subvolume.has_value());

	auto const timestamp = current_timestamp();
	auto snapshot_result = subvolume->take_snapshot(timestamp);
	ASSERT(snapshot_result.has_value());

	using Entry = SnapshotList::Entry;
	auto list = SnapshotList{*subvolume};
	auto const entries = list.get_entries();
	EXPECT(!entries.empty());
	auto const it = std::ranges::find_if(entries, [&](Entry const& e) { return e.snapshot.path == snapshot_result->path; });
	ASSERT(it != entries.end());

	auto delete_result = list.delete_snapshot(it->number);
	EXPECT(delete_result.has_value());
	EXPECT(list.get_entries().empty());
}
} // namespace
} // namespace btrsnap::test
