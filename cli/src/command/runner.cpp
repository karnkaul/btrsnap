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
	for (auto const& subcommand : subcommands) {
		spec.commands.push_back(clap::command(subcommand->get_identifier(), subcommand->get_parameter_list(), subcommand->get_help_text()));
	}

	auto parser = clap::Parser{std::move(spec)};
	auto const parse_result = parser.parse_main(argc, argv);
	if (parse_result.should_early_exit()) { return static_cast<ExitCode>(parse_result.exit_code()); }

	m_command_identifier = parse_result.command_identifier();
	return {};
}

auto Runner::execute(SubvolumeList& subvolume_list) -> ExitCode { return get_command(m_command_identifier).execute(subvolume_list); }

auto Runner::get_command(std::string_view const identifier) const -> ICommand& {
	auto const it = std::ranges::find_if(subcommands, [identifier](auto const& sc) { return sc->get_identifier() == identifier; });
	KLIB_ASSERT(it != subcommands.end());
	return **it;
}
} // namespace btrsnap::cli
