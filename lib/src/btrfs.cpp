#include "btrsnap/btrfs.hpp"
#include "detail/to_error.hpp"
#include <format>
#include <utility>

#if !__has_include(<btrfsutil.h>)
#error btrfsutil.h not found
#endif

#include <btrfsutil.h>

namespace btrsnap {
namespace btrfs {
namespace {
template <typename F>
auto wrap(F func) -> Result<void> {
	auto const result = func();
	if (result == BTRFS_UTIL_OK) { return {}; }

	auto const message = std::format("{} ({})", btrfs_util_strerror(result), std::exchange(errno, 0));
	return detail::to_error(Error::Type::Btrfs, message);
}
} // namespace
} // namespace btrfs

auto btrfs::is_subvolume(klib::CString const path) -> Result<void> {
	return wrap([&] { return btrfs_util_subvolume_is_valid(path.c_str()); });
}

auto btrfs::create_snapshot(klib::CString const src, klib::CString const dst) -> Result<void> {
	return wrap([&] { return btrfs_util_subvolume_snapshot(src.c_str(), dst.c_str(), 0, nullptr, nullptr); });
}

auto btrfs::delete_subvolume(klib::CString const path) -> Result<void> {
	return wrap([&] { return btrfs_util_subvolume_delete(path.c_str(), 0); });
}
} // namespace btrsnap
