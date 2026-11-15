export module btrsnap.types;

export import klib.core;

export namespace btrsnap {
using namespace std::chrono_literals;

namespace fs = std::filesystem;

using Clock = std::chrono::system_clock;
using Seconds = std::chrono::seconds;
using Timestamp = std::chrono::seconds;

enum struct SnapshotNumber : int {};

enum class ErrorType : std::int8_t {
	Unknown,
	InvalidArgument,
	Filesystem,
	Btrfs,
};
inline auto const error_type_name_map = klib::EnumNameMap<ErrorType>{
	{ErrorType::Unknown, "UnknownError"},
	{ErrorType::InvalidArgument, "InvalidArgument"},
	{ErrorType::Filesystem, "Filesystem"},
	{ErrorType::Btrfs, "BtrfsError"},
};

struct Error {
	using Type = ErrorType;

	Type type{};
	std::string message{};
};

template <typename Type>
using Result = std::expected<Type, Error>;

struct Snapshot {
	fs::path path{};
	Timestamp timestamp{};
};

struct RecycleReport {
	void append(RecycleReport other) {
		archived.append_range(std::move(other.archived));
		deleted.append_range(std::move(other.deleted));
	}

	std::vector<Snapshot> archived{};
	std::vector<Result<Snapshot>> deleted{};
};

[[nodiscard]] auto to_error(Error::Type type, std::string_view message) -> std::unexpected<Error>;
[[nodiscard]] auto filesystem_error(std::string_view message) -> std::unexpected<Error> {
	return to_error(Error::Type::Filesystem, message);
}

[[nodiscard]] auto to_timestamp(klib::CString serialized) -> std::optional<Timestamp>;
[[nodiscard]] auto current_timestamp() -> Timestamp;
[[nodiscard]] auto to_pathname(Timestamp timestamp) -> std::string;
[[nodiscard]] auto format_delta_time(Seconds delta_time, int num_largest = 2) -> std::string;
} // namespace btrsnap
