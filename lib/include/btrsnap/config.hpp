#pragma once
#include "klib/string/c_string.hpp"
#include <optional>
#include <vector>

namespace btrsnap {
struct Config {
	static constexpr std::string_view directory_v{"/etc/btrsnap"};

	[[nodiscard]] static auto from_file(klib::CString path) -> std::optional<Config>;
	[[nodiscard]] static auto from_directory(std::string_view directory = directory_v) -> std::vector<Config>;

	void print(std::string_view save_directory = directory_v) const;

	std::string subvolume{};
	std::string subdirectory{".snapshots"};
	int limit{3};
};
} // namespace btrsnap
