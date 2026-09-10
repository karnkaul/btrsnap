#pragma once
#include "btrsnap/btrfs.hpp"
#include <filesystem>

namespace btrsnap::test {
namespace fs = std::filesystem;

class MockBtrfs : public IBtrfs {
  public:
	[[nodiscard]] static auto is_directory(fs::path const& path) -> Result<void>;
	[[nodiscard]] static auto create_directory(fs::path const& path) -> Result<void>;
	[[nodiscard]] static auto delete_directory(fs::path const& path) -> Result<void>;

	[[nodiscard]] auto is_subvolume(klib::CString const path) const -> Result<void> final { return is_directory(path.as_view()); }
	[[nodiscard]] auto create_snapshot(klib::CString /*src*/, klib::CString const dst) const -> Result<void> final { return create_directory(dst.as_view()); }
	[[nodiscard]] auto create_subvolume(klib::CString const path) const -> Result<void> final { return create_directory(path.as_view()); }
	[[nodiscard]] auto delete_subvolume(klib::CString const path) const -> Result<void> final { return delete_directory(path.as_view()); }
};
} // namespace btrsnap::test
