#include "detail/btrfs_impl.hpp"
#include "btrsnap/btrfs.hpp"
#include "detail/to_error.hpp"
#include <format>

#if !__has_include(<btrfsutil.h>)
#error btrfsutil.h not found
#endif

#include <btrfsutil.h>

namespace btrsnap {
namespace detail {
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

class Btrfs : public IBtrfs {
	[[nodiscard]] auto is_subvolume(klib::CString const path) const -> Result<void> final {
		auto const resetter = ErrnoResetter{};
		auto const result = ::btrfs_util_subvolume_is_valid(path.c_str());
		if (result == BTRFS_UTIL_OK) { return {}; }
		return format_error(result, path.as_view());
	}

	[[nodiscard]] auto create_snapshot(klib::CString const src, klib::CString const dst) const -> Result<void> final {
		auto const resetter = ErrnoResetter{};
		auto const result = ::btrfs_util_subvolume_snapshot(src.c_str(), dst.c_str(), 0, nullptr, nullptr);
		if (result == BTRFS_UTIL_OK) { return {}; }
		return format_error(result, dst.as_view());
	}

	[[nodiscard]] auto create_subvolume(klib::CString const path) const -> Result<void> final {
		auto const resetter = ErrnoResetter{};
		auto const result = ::btrfs_util_subvolume_create(path.c_str(), 0, nullptr, nullptr);
		if (result == BTRFS_UTIL_OK) { return {}; }
		return format_error(result, path.as_view());
	}

	[[nodiscard]] auto delete_subvolume(klib::CString const path) const -> Result<void> final {
		auto const resetter = ErrnoResetter{};
		auto const result = ::btrfs_util_subvolume_delete(path.c_str(), 0);
		if (result == BTRFS_UTIL_OK) { return {}; }
		return format_error(result, path.as_view());
	}
};

auto const g_default_btrfs = Btrfs{};
// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
auto g_btrfs = klib::Ptr<IBtrfs const>{&g_default_btrfs};
} // namespace
} // namespace detail

void detail::set_btrfs(klib::Ptr<IBtrfs const> btrfs) {
	if (!btrfs) { btrfs = &g_default_btrfs; }
	g_btrfs = btrfs;
}

auto btrfs::is_subvolume(klib::CString const path) -> Result<void> { return detail::g_btrfs->is_subvolume(path); }
auto btrfs::create_snapshot(klib::CString const src, klib::CString const dst) -> Result<void> { return detail::g_btrfs->create_snapshot(src, dst); }
auto btrfs::create_subvolume(klib::CString const path) -> Result<void> { return detail::g_btrfs->create_subvolume(path); }
auto btrfs::delete_subvolume(klib::CString const path) -> Result<void> { return detail::g_btrfs->delete_subvolume(path); }
} // namespace btrsnap
