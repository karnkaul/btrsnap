#pragma once
#include "detail/btrfs_impl.hpp"
#include "detail/to_error.hpp"
#include <filesystem>
#include <format>

namespace btrsnap::test {
namespace fs = std::filesystem;

class MockBtrfs : public detail::IBtrfs {
  public:
	MockBtrfs(MockBtrfs const&) = delete;
	MockBtrfs(MockBtrfs&&) = delete;
	MockBtrfs& operator=(MockBtrfs const&) = delete;
	MockBtrfs& operator=(MockBtrfs&&) = delete;

	explicit MockBtrfs() { detail::set_btrfs(this); }
	~MockBtrfs() { detail::set_btrfs(nullptr); }

	[[nodiscard]] static auto is_directory(fs::path const& path) -> Result<void> {
		if (fs::is_directory(path)) { return {}; }
		return detail::to_error(Error::Type::Btrfs, std::format("Not a directory: {}", path.generic_string()));
	}

	[[nodiscard]] static auto create_directory(fs::path const& path) -> Result<void> {
		if (fs::exists(path)) { return detail::to_error(Error::Type::Btrfs, std::format("Already exists: {}", path.generic_string())); }
		if (!fs::create_directories(path)) {
			return detail::to_error(Error::Type::Btrfs, std::format("Failed to create directory: {}", path.generic_string()));
		}
		return {};
	}

	[[nodiscard]] static auto delete_directory(fs::path const& path) -> Result<void> {
		if (!fs::is_directory(path)) { return detail::to_error(Error::Type::Btrfs, std::format("Not a directory: {}", path.generic_string())); }
		if (!fs::remove_all(path)) { return detail::to_error(Error::Type::Btrfs, std::format("Failed to delete directory: {}", path.generic_string())); }
		return {};
	}

	[[nodiscard]] auto is_subvolume(klib::CString const path) const -> Result<void> final { return is_directory(path.as_view()); }
	[[nodiscard]] auto create_snapshot(klib::CString /*src*/, klib::CString const dst) const -> Result<void> final { return create_directory(dst.as_view()); }
	[[nodiscard]] auto create_subvolume(klib::CString const path) const -> Result<void> final { return create_directory(path.as_view()); }
	[[nodiscard]] auto delete_subvolume(klib::CString const path) const -> Result<void> final { return delete_directory(path.as_view()); }
};
} // namespace btrsnap::test
