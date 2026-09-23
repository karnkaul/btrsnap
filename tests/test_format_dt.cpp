#include "btrsnap/clock.hpp"
#include "klib/unit_test/unit_test.hpp"

namespace btrsnap::test {
namespace {
using namespace std::chrono_literals;

TEST_CASE(format_delta_time) {
	auto dt = 4h + 30min;
	auto str = format_delta_time(dt);
	EXPECT(str == "4h 30min");

	dt = 26h;
	str = format_delta_time(dt);
	EXPECT(str == "1d 2h");

	dt = 6 * 24h;
	str = format_delta_time(dt);
	EXPECT(str == "6d 0h");

	dt = 8 * 24h;
	str = format_delta_time(dt);
	EXPECT(str == "1w 1d");

	dt = (30 * 24h) + (8 * 24h);
	str = format_delta_time(dt);
	EXPECT(str == "1m 1w");

	dt = (365 * 24h) + (40 * 24h);
	str = format_delta_time(dt);
	EXPECT(str == "1y 1m");
}
} // namespace
} // namespace btrsnap::test
