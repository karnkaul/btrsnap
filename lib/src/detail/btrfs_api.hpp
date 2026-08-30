#pragma once
#include "klib/string/c_string.hpp"
#include <expected>

namespace btrsnap::detail::btrfs {
struct Error {
	int code{};
	klib::CString text{};
};

using Result = std::expected<void, Error>;

[[nodiscard]] auto is_subvolume(klib::CString path) -> Result;
[[nodiscard]] auto create_snapshot(klib::CString src, klib::CString dst) -> Result;
auto delete_subvolume(klib::CString path) -> Result;
} // namespace btrsnap::detail::btrfs
