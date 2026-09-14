#pragma once
#include "klib/string/c_string.hpp"
#include <chrono>
#include <optional>
#include <vector>

namespace btrsnap {
struct Config {
	static constexpr std::string_view directory_v{"/etc/btrsnap"};

	[[nodiscard]] static auto from_file(klib::CString path) -> std::optional<Config>;
	[[nodiscard]] static auto from_directory(std::string_view directory = directory_v) -> std::vector<Config>;

	void print(std::string_view save_directory = directory_v) const;

	std::string subvolume{};
	std::string snapshots_subdirectory{".snapshots"};
	int snapshot_limit{3};
	std::string archive_subdirectory{".archive"};
	std::chrono::days archive_period{7};
	int archive_limit{3};
};
} // namespace btrsnap
