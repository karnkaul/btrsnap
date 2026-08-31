#pragma once
#include "btrsnap/result.hpp"
#include "klib/string/c_string.hpp"

namespace btrsnap::btrfs {
[[nodiscard]] auto is_subvolume(klib::CString path) -> Result<void>;
[[nodiscard]] auto create_snapshot(klib::CString src, klib::CString dst) -> Result<void>;
[[nodiscard]] auto create_subvolume(klib::CString path) -> Result<void>;
[[nodiscard]] auto delete_subvolume(klib::CString path) -> Result<void>;
} // namespace btrsnap::btrfs
