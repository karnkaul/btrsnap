#pragma once
#include <cstdint>
#include <cstdlib>

namespace btrsnap::cli {
enum struct ExitCode : std::int8_t {
	Success = EXIT_SUCCESS,
	Failure = EXIT_FAILURE,
};
} // namespace btrsnap::cli
