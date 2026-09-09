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

auto btrsnap::to_timestamp(klib::CString const serialized) -> std::optional<Timestamp> {
	auto str = std::istringstream{serialized.c_str()};
	auto ret = std::chrono::time_point<Clock, Timestamp>{};
	if (str >> std::chrono::parse("%Y-%m-%d_%H-%M-%S", ret)) { return ret.time_since_epoch(); }
	return {};
}

auto btrsnap::current_timestamp() -> Timestamp {
	auto const clamped = std::chrono::time_point_cast<Timestamp>(Clock::now());
	return std::chrono::zoned_time{std::chrono::current_zone(), clamped}.get_local_time().time_since_epoch();
}

auto btrsnap::to_pathname(Timestamp const timestamp) -> std::string {
	auto const time_point = std::chrono::time_point<Clock, Timestamp>{timestamp};
	return std::format("{:%Y-%m-%d_%H-%M-%S}", time_point);
}

auto btrsnap::format_delta_time(Seconds const delta_time, int const num_largest) -> std::string { return DurationSplitter{}(delta_time, num_largest); }
