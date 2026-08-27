#pragma once
#include <filesystem>

namespace btrsnap::detail {
namespace fs = std::filesystem;

struct FileInfo {
	fs::path path{};
	fs::file_time_type last_modified{};
};
} // namespace btrsnap::detail
