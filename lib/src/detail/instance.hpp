#pragma once
#include "detail/agent.hpp"
#include "detail/config.hpp"
#include "klib/enum/bitops.hpp"
#include <optional>

namespace btrsnap::detail {
enum class Flag : std::int8_t {
	None = 0,
	NoTrim = 1 << 0,
};

[[nodiscard]] constexpr auto enable_enum_bitops(Flag /*unused*/) { return true; }

class Instance {
  public:
	[[nodiscard]] static auto create() -> std::optional<Instance>;

	void list_snapshots();
	auto take_snapshots(Flag flags = Flag::None) -> bool;
	void trim_snapshots();
	auto clear_snapshots() -> bool;

  private:
	explicit Instance(std::span<Config const> configs);

	std::vector<Agent> m_agents{};
};
} // namespace btrsnap::detail
