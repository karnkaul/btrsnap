#include "btrsnap/config.hpp"
#include "common/test_directory.hpp"
#include "klib/file_io.hpp"
#include "klib/unit_test/unit_test.hpp"

namespace btrsnap::test {
namespace {
TEST_CASE(config_from_file) {
	auto const test_dir = TestDirectory{};
	auto const config_path = (test_dir / "test.conf").string();
	static constexpr std::string_view text_v = R"(
SUBVOLUME=subvolume
SUBDIRECTORY=subdirectory
LIMIT=5
)";
	ASSERT(klib::write_to_file(text_v, config_path));

	auto const config = Config::from_file(config_path);
	ASSERT(config.has_value());
	EXPECT(config->subvolume == "subvolume");
	EXPECT(config->subdirectory == "subdirectory");
	EXPECT(config->limit == 5);
}
} // namespace
} // namespace btrsnap::test
