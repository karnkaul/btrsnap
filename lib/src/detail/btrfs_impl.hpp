#pragma once
#include "btrsnap/result.hpp"
#include "klib/base_types.hpp"
#include "klib/ptr.hpp"
#include "klib/string/c_string.hpp"

namespace btrsnap::detail {
class IBtrfs : public klib::Polymorphic {
  public:
	[[nodiscard]] virtual auto is_subvolume(klib::CString path) const -> Result<void> = 0;
	[[nodiscard]] virtual auto create_snapshot(klib::CString src, klib::CString dst) const -> Result<void> = 0;
	[[nodiscard]] virtual auto create_subvolume(klib::CString path) const -> Result<void> = 0;
	[[nodiscard]] virtual auto delete_subvolume(klib::CString path) const -> Result<void> = 0;
};

void set_btrfs(klib::Ptr<IBtrfs const> btrfs);
} // namespace btrsnap::detail
