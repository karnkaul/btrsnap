#include "btrsnap/util.hpp"
#include "djson/json.hpp"
#include "klib/unit_test/unit_test.hpp"

namespace btrsnap::test {
namespace {
using namespace std::chrono_literals;

TEST_CASE(instance_info_parse) {
	static constexpr std::string_view json_v = R"({
	"storage": "/root",
	"subvolumes": [
		{
			"name": "@home",
			"path": "/home",
			"recycle": {
				"snapshot_limit": 2,
				"archive_period_days": 3,
				"archive_limit": 2
			}
		}
	]
})";

	auto instance_info = InstanceInfo{};
	auto json = dj::Json::parse(json_v);
	ASSERT(json);
	util::from_json(*json, instance_info);

	EXPECT(instance_info.storage.root == "/root");

	ASSERT(instance_info.subvolumes.size() == 1);
	auto const& subvolume = instance_info.subvolumes.front();
	EXPECT(subvolume.name == "@home");
	EXPECT(subvolume.path == "/home");
	EXPECT(subvolume.recycle.snapshot_limit == 2);
	EXPECT(subvolume.recycle.archive_period == 3 * 24h);
	EXPECT(subvolume.recycle.archive_limit == 2);
}
} // namespace
} // namespace btrsnap::test
