#include "detail/to_error.hpp"
#include <format>

namespace btrsnap {
auto detail::to_error(Error::Type const type, std::string_view const message) -> std::unexpected<Error> {
	return std::unexpected{Error{
		.type = type,
		.message = std::format("{}: {}", error_type_name_map.to_name(type), message),
	}};
}
} // namespace btrsnap
