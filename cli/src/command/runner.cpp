#include "command/runner.hpp"
#include "btrsnap/build_version.hpp"
#include "clap/parser.hpp"
#include "klib/debug/assert.hpp"
#include <algorithm>

namespace btrsnap::cli {
auto Runner::parse_args(clap::OptionList options, int const argc, char const* const* argv) -> std::optional<ExitCode> {
	static auto const version_str = std::format("{}", build_version_v);
	auto spec = clap::spec::Commands{
		.options = std::move(options),
		.program =
			{
				.name = "btrsnap",
				.version = version_str,
			},
	};
	for (auto const& subcommand : commands) {
		spec.commands.push_back(clap::command(subcommand->get_identifier(), subcommand->get_parameter_list(), subcommand->get_help_text()));
	}

	auto parser = clap::Parser{std::move(spec)};
	auto const parse_result = parser.parse_main(argc, argv);
	if (parse_result.should_early_exit()) { return static_cast<ExitCode>(parse_result.exit_code()); }

	m_command_identifier = parse_result.command_identifier();
	return {};
}

auto Runner::execute(SubvolumeList& subvolume_list) -> ExitCode {
	auto const it = std::ranges::find_if(commands, [this](auto const& cmd) { return cmd->get_identifier() == m_command_identifier; });
	KLIB_ASSERT(it != commands.end());
	auto& command = *it;
	return command->execute(subvolume_list);
}
} // namespace btrsnap::cli
