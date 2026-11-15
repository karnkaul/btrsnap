export module btrsnap.test;

export import btrsnap;

export namespace btrsnap::test {
namespace fs = std::filesystem;

class TestDirectory {
  public:
	TestDirectory(TestDirectory const&) = delete;
	TestDirectory(TestDirectory&&) = delete;
	TestDirectory& operator=(TestDirectory const&) = delete;
	TestDirectory& operator=(TestDirectory&&) = delete;

	explicit TestDirectory(fs::path const& path = ".test") : m_path(fs::absolute(path)) {
		if (fs::exists(m_path)) {
			fs::remove_all(m_path);
		}
		if (!fs::create_directories(m_path)) {
			throw std::runtime_error{std::format("Failed to create test directory: {}", m_path.generic_string())};
		}
	}

	~TestDirectory() { fs::remove_all(m_path); }

	[[nodiscard]] auto get_path() const -> fs::path const& { return m_path; }

  private:
	fs::path m_path{};
};

[[nodiscard]] auto operator/(TestDirectory const& test_dir, fs::path const& path) -> fs::path {
	return test_dir.get_path() / path;
}

class MockBtrfs : public IBtrfs {
  public:
	[[nodiscard]] static auto is_directory(fs::path const& path) -> Result<void> {
		if (fs::is_directory(path)) {
			return {};
		}
		return to_error(Error::Type::Btrfs, std::format("Not a directory: {}", path.generic_string()));
	}

	[[nodiscard]] static auto create_directory(fs::path const& path) -> Result<void> {
		if (fs::exists(path)) {
			return to_error(Error::Type::Btrfs, std::format("Already exists: {}", path.generic_string()));
		}
		if (!fs::create_directories(path)) {
			return to_error(Error::Type::Btrfs, std::format("Failed to create directory: {}", path.generic_string()));
		}
		return {};
	}

	[[nodiscard]] static auto delete_directory(fs::path const& path) -> Result<void> {
		if (!fs::is_directory(path)) {
			return to_error(Error::Type::Btrfs, std::format("Not a directory: {}", path.generic_string()));
		}
		if (!fs::remove_all(path)) {
			return to_error(Error::Type::Btrfs, std::format("Failed to delete directory: {}", path.generic_string()));
		}
		return {};
	}

	[[nodiscard]] static auto copy_directory(fs::path const& src, fs::path const& dst) -> Result<void> {
		if (!fs::is_directory(src)) {
			return to_error(Error::Type::Btrfs, std::format("Not a directory: {}", src.generic_string()));
		}
		if (dst.has_parent_path()) {
			auto const parent = dst.parent_path();
			if (!fs::is_directory(parent)) {
				auto result = create_directory(parent);
				if (!result) {
					return std::unexpected{std::move(result.error())};
				}
			}
		}
		auto err = std::error_code{};
		fs::copy(src, dst, err);
		if (err != std::error_code{}) {
			return to_error(
				Error::Type::Btrfs,
				std::format("Failed to copy directory: {} to: {}", src.generic_string(), dst.generic_string()));
		}
		return {};
	}

	[[nodiscard]] auto is_subvolume(klib::CString const path) const -> Result<void> final {
		return is_directory(path.as_view());
	}

	[[nodiscard]] auto create_snapshot(klib::CString const src, klib::CString const dst) const -> Result<void> final {
		return copy_directory(src.as_view(), dst.as_view());
	}

	[[nodiscard]] auto create_readonly_snapshot(klib::CString src, klib::CString const dst) const
		-> Result<void> final {
		return create_snapshot(src, dst);
	}
	[[nodiscard]] auto create_subvolume(klib::CString const path) const -> Result<void> final {
		return create_directory(path.as_view());
	}
	[[nodiscard]] auto delete_subvolume(klib::CString const path) const -> Result<void> final {
		return delete_directory(path.as_view());
	}
};

class Environment {
  public:
	static constexpr std::string_view snapshots_subdirectory_v{".snapshots"};
	static constexpr std::string_view archive_subdirectory_v{".archive"};

	[[nodiscard]] auto get_btrfs() const -> IBtrfs const& { return m_btrfs; }
	[[nodiscard]] auto get_test_directory() const -> fs::path const& { return m_test_dir.get_path(); }
	[[nodiscard]] auto get_storage_config() const -> StorageConfig const& { return m_storage_config; }

	[[nodiscard]] auto path_to(std::string_view const subpath) const -> fs::path {
		return get_test_directory() / subpath;
	}

	[[nodiscard]] auto create_subvolume_config(std::string_view subvolume_subpath,
											   std::string_view name = "subvol",
											   RecycleConfig recycle = {}) const -> SubvolumeConfig {
		return SubvolumeConfig{
			.name = std::string{name},
			.path = path_to(subvolume_subpath),
			.recycle = recycle,
		};
	}

	[[nodiscard]] auto create_subvolume(std::string_view subpath,
										std::string_view name = "subvol",
										RecycleConfig recycle = {}) const -> Result<Subvolume> {
		auto path = path_to(subpath);
		if (!fs::is_directory(path)) {
			auto const result = MockBtrfs::create_directory(path.generic_string());
			if (!result) {
				throw std::runtime_error{result.error().message};
			}
		}

		auto subvolume_config = create_subvolume_config(subpath, name, recycle);
		return Subvolume::create(&get_btrfs(), get_storage_config(), std::move(subvolume_config));
	}

  private:
	TestDirectory m_test_dir{};
	MockBtrfs m_btrfs{};
	StorageConfig m_storage_config{.root = m_test_dir.get_path() / "storage"};
};
} // namespace btrsnap::test
