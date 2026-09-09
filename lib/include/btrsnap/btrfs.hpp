#pragma once
#include "btrsnap/result.hpp"
#include "klib/base_types.hpp"
#include "klib/string/c_string.hpp"

namespace btrsnap {
class IBtrfs : public klib::Polymorphic {
  public:
	[[nodiscard]] static auto get_default() -> IBtrfs const&;

	[[nodiscard]] virtual auto is_subvolume(klib::CString path) const -> Result<void> = 0;
	[[nodiscard]] virtual auto create_snapshot(klib::CString src, klib::CString dst) const -> Result<void> = 0;
	[[nodiscard]] virtual auto create_subvolume(klib::CString path) const -> Result<void> = 0;
	[[nodiscard]] virtual auto delete_subvolume(klib::CString path) const -> Result<void> = 0;
};
} // namespace btrsnap
