#pragma once
#include <chrono>
#include <filesystem>
#include <string>
#include <vector>

namespace btrsnap {
namespace fs = std::filesystem;

struct StorageConfig {
	static constexpr std::string_view snapshots_subdirectory_v{"snapshots"};
	static constexpr std::string_view archive_subdirectory_v{"archive"};

	[[nodiscard]] auto get_snapshots_path() const -> fs::path { return root / snapshots_subdirectory_v; }
	[[nodiscard]] auto get_archive_path() const -> fs::path { return root / archive_subdirectory_v; }

	fs::path root{"/btrsnap"};
};

struct RecycleConfig {
	int snapshot_limit{3};
	int archive_limit{3};
	std::chrono::days archive_period{7};
};

struct SubvolumeConfig {
	std::string name;
	fs::path path;

	RecycleConfig recycle{};
};

struct InstanceConfig {
	StorageConfig storage{};
	std::vector<SubvolumeConfig> subvolumes{};
};
} // namespace btrsnap
