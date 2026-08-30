#pragma once
#include "detail/config.hpp"
#include "detail/subvolume.hpp"
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
	[[nodiscard]] static auto create(klib::CString custom_config_path) -> std::optional<Instance>;

	void list_snapshots();
	auto take_snapshots(Flag flags = Flag::None) -> bool;
	void trim_snapshots();
	auto clear_snapshots() -> bool;

  private:
	[[nodiscard]] static auto create_impl(std::span<Config const> configs) -> std::optional<Instance>;

	explicit Instance(std::span<Config const> configs);

	template <typename F>
	auto delete_snapshots(F get_keep) -> bool;

	std::vector<Subvolume> m_subvolumes{};
};
} // namespace btrsnap::detail
