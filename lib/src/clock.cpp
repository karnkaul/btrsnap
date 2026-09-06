#include "btrsnap/clock.hpp"
#include <format>
#include <sstream>

namespace btrsnap {
namespace {
using namespace std::chrono_literals;

class DurationSplitter {
  public:
	[[nodiscard]] auto operator()(Seconds const in, int const num_largest) -> std::string {
		m_remain = in;
		m_to_append = num_largest;

		subtract<std::chrono::years>();
		subtract<std::chrono::months>();
		subtract<std::chrono::weeks>();
		subtract<std::chrono::days>();
		subtract<std::chrono::hours>();
		subtract<std::chrono::minutes>();
		subtract<std::chrono::seconds>();
		return std::move(m_ret);
	}

  private:
	template <typename DurationT>
	void subtract() {
		if (m_to_append <= 0) { return; }
		auto const duration = std::chrono::duration_cast<DurationT>(m_remain);
		if (m_ret.empty() && duration == 0s) { return; }
		m_remain -= duration;
		if (!m_ret.empty()) { m_ret.push_back(' '); }
		std::format_to(std::back_inserter(m_ret), "{}", duration);
		--m_to_append;
	}

	Seconds m_remain{};
	std::string m_ret{};
	int m_to_append{};
};
} // namespace
} // namespace btrsnap

auto btrsnap::to_timestamp(klib::CString const timestamp) -> std::optional<Clock::time_point> {
	auto str = std::istringstream{timestamp.c_str()};
	auto ret = Clock::time_point{};
	if (str >> std::chrono::parse("%Y-%m-%d_%H-%M-%S", ret)) { return ret; }
	return {};
}

auto btrsnap::to_zoned_seconds(Clock::time_point const& timestamp) -> ZonedSeconds {
	auto const clamped = std::chrono::time_point_cast<Seconds>(timestamp);
	return std::chrono::zoned_time{std::chrono::current_zone(), clamped};
}

auto btrsnap::to_pathname(ZonedSeconds const& zoned_seconds) -> std::string { return std::format("{:%Y-%m-%d_%H-%M-%S}", zoned_seconds); }

auto btrsnap::format_delta_time(ZonedSeconds const& now, Clock::time_point const& timestamp, int const num_largest) -> std::string {
	auto const dt = now.get_local_time() - std::chrono::local_time{timestamp.time_since_epoch()};
	return DurationSplitter{}(std::chrono::duration_cast<Seconds>(dt), num_largest);
}
