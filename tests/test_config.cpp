#include "btrsnap/config.hpp"
#include "common/test_directory.hpp"
#include "klib/file_io.hpp"
#include "klib/unit_test/unit_test.hpp"

namespace btrsnap::test {
namespace {
using namespace std::chrono_literals;

TEST_CASE(config_from_file) {
	auto const test_dir = TestDirectory{};
	auto const config_path = (test_dir / "test.conf").generic_string();
	static constexpr std::string_view text_v = R"(
SUBVOLUME=subvolume
SNAPSHOTS_SUBDIRECTORY=snapshots
SNAPSHOT_LIMIT=5
ARCHIVE_SUBDIRECTORY=archive
ARCHIVE_PERIOD=14
ARCHIVE_LIMIT=2
)";
	ASSERT(klib::write_to_file(text_v, config_path));

	auto const config = Config::from_file(config_path);
	ASSERT(config.has_value());
	EXPECT(config->subvolume == "subvolume");
	EXPECT(config->snapshots_subdirectory == "snapshots");
	EXPECT(config->snapshot_limit == 5);
	EXPECT(config->archive_subdirectory == "archive");
	EXPECT(config->archive_period == 7 * 24h);
	EXPECT(config->archive_limit == 2);
}
} // namespace
} // namespace btrsnap::test
