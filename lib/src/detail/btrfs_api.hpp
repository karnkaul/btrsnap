#pragma once
#include "klib/string/c_string.hpp"
#include <expected>

namespace btrsnap::detail::btrfs {
struct Error {
	int code{};
	klib::CString text{};
};

template <typename Type>
using Result = std::expected<Type, Error>;

[[nodiscard]] auto is_subvolume(klib::CString path) -> Result<void>;
} // namespace btrsnap::detail::btrfs
