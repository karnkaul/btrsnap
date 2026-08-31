#include "btrsnap/btrfs.hpp"
#include "detail/to_error.hpp"
#include <format>

#if !__has_include(<btrfsutil.h>)
#error btrfsutil.h not found
#endif

#include <btrfsutil.h>

namespace btrsnap {
namespace btrfs {
namespace {
class ErrnoResetter {
  public:
	ErrnoResetter(ErrnoResetter const&) = delete;
	ErrnoResetter(ErrnoResetter&&) = delete;
	ErrnoResetter& operator=(ErrnoResetter const&) = delete;
	ErrnoResetter& operator=(ErrnoResetter&&) = delete;

	ErrnoResetter() = default;
	~ErrnoResetter() { errno = 0; }
};

[[nodiscard]] auto format_error(btrfs_util_error const result, std::string_view message) {
	auto const formatted = std::format("{}: {} ({})", message, ::btrfs_util_strerror(result), errno);
	return detail::to_error(Error::Type::Btrfs, formatted);
}
} // namespace
} // namespace btrfs

auto btrfs::is_subvolume(klib::CString const path) -> Result<void> {
	auto const resetter = ErrnoResetter{};
	auto const result = ::btrfs_util_subvolume_is_valid(path.c_str());
	if (result == BTRFS_UTIL_OK) { return {}; }
	return format_error(result, path.as_view());
}

auto btrfs::create_snapshot(klib::CString const src, klib::CString const dst) -> Result<void> {
	auto const resetter = ErrnoResetter{};
	auto const result = ::btrfs_util_subvolume_snapshot(src.c_str(), dst.c_str(), 0, nullptr, nullptr);
	if (result == BTRFS_UTIL_OK) { return {}; }
	return format_error(result, dst.as_view());
}

auto btrfs::delete_subvolume(klib::CString const path) -> Result<void> {
	auto const resetter = ErrnoResetter{};
	auto const result = ::btrfs_util_subvolume_delete(path.c_str(), 0);
	if (result == BTRFS_UTIL_OK) { return {}; }
	return format_error(result, path.as_view());
}
} // namespace btrsnap
