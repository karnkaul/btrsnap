#pragma once
#include <filesystem>
#include <format>
#include <stdexcept>

namespace btrsnap::test {
namespace fs = std::filesystem;

class TestDirectory {
  public:
	TestDirectory(TestDirectory const&) = delete;
	TestDirectory(TestDirectory&&) = delete;
	TestDirectory& operator=(TestDirectory const&) = delete;
	TestDirectory& operator=(TestDirectory&&) = delete;

	explicit TestDirectory(fs::path const& path = ".test") : m_path(fs::absolute(path)) {
		if (fs::exists(m_path)) { fs::remove_all(m_path); }
		if (!fs::create_directories(m_path)) { throw std::runtime_error{std::format("Failed to create test directory: {}", m_path.generic_string())}; }
	}

	~TestDirectory() { fs::remove_all(m_path); }

	[[nodiscard]] auto get_path() const -> fs::path const& { return m_path; }

  private:
	fs::path m_path{};
};

[[nodiscard]] inline auto operator/(TestDirectory const& test_dir, fs::path const& path) -> fs::path { return test_dir.get_path() / path; }
} // namespace btrsnap::test
