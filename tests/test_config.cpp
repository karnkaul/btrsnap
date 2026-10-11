#include "btrsnap/util.hpp"
#include "djson/json.hpp"
#include "klib/unit_test/unit_test.hpp"

namespace btrsnap::test {
namespace {
using namespace std::chrono_literals;

TEST_CASE(instance_config_parse) {
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

	auto instance_config = InstanceConfig{};
	auto json = dj::Json::parse(json_v);
	ASSERT(json);
	util::from_json(*json, instance_config);

	EXPECT(instance_config.storage.root == "/root");

	ASSERT(instance_config.subvolumes.size() == 1);
	auto const& subvolume = instance_config.subvolumes.front();
	EXPECT(subvolume.name == "@home");
	EXPECT(subvolume.path == "/home");
	EXPECT(subvolume.recycle.snapshot_limit == 2);
	EXPECT(subvolume.recycle.archive_period == 3 * 24h);
	EXPECT(subvolume.recycle.archive_limit == 2);
}
} // namespace
} // namespace btrsnap::test
