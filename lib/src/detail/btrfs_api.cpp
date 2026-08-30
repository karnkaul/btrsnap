#include "detail/btrfs_api.hpp"
#include "klib/debug/assert.hpp"

#if !__has_include(<btrfsutil.h>)
#error btrfsutil.h not found
#endif

#include <btrfsutil.h>

namespace btrsnap::detail {
namespace btrfs {
namespace {
[[nodiscard]] auto to_error(btrfs_util_error const in) -> Error {
	KLIB_ASSERT(in != BTRFS_UTIL_OK);
	auto const ret = Error{
		.code = errno,
		.text = btrfs_util_strerror(in),
	};
	errno = {};
	return ret;
}

template <typename F>
auto wrap(F func) -> Result {
	auto const result = func();
	if (result == BTRFS_UTIL_OK) { return {}; }
	return std::unexpected{to_error(result)};
}
} // namespace
} // namespace btrfs

auto btrfs::is_subvolume(klib::CString const path) -> Result {
	return wrap([&] { return btrfs_util_subvolume_is_valid(path.c_str()); });
}

auto btrfs::create_snapshot(klib::CString src, klib::CString dst) -> Result {
	return wrap([&] { return btrfs_util_subvolume_snapshot(src.c_str(), dst.c_str(), 0, nullptr, nullptr); });
}

auto btrfs::delete_subvolume(klib::CString const path) -> Result {
	return wrap([&] { return btrfs_util_subvolume_delete(path.c_str(), 0); });
}
} // namespace btrsnap::detail
