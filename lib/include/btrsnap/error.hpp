#pragma once
#include "klib/enum/map.hpp"
#include <cstdint>
#include <string>

namespace btrsnap {
enum class ErrorType : std::int8_t {
	Unknown,
	InvalidArgument,
	Btrfs,
};
inline auto const error_type_name_map = klib::EnumNameMap<ErrorType>{
	{ErrorType::Unknown, "UnknownError"},
	{ErrorType::InvalidArgument, "InvalidArgument"},
	{ErrorType::Btrfs, "BtrfsError"},
};

struct Error {
	using Type = ErrorType;

	Type type{};
	std::string message{};
};
} // namespace btrsnap
