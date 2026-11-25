module btrsnap.cli;

import clap;

namespace btrsnap::cli {
namespace {
[[nodiscard]] auto all_success(std::span<Result<Snapshot> const> results) {
	return std::ranges::all_of(results, [](auto const& result) {
		return result.has_value();
	});
}

class TakeSnapshot : public ICommand {
	[[nodiscard]] auto get_identifier() const -> std::string_view final { return "snapshot"; }
	[[nodiscard]] auto get_help_text() const -> std::string_view final { return "take and/or recycle snapshots"; }

	auto get_parameter_list() -> clap::ParameterList final {
		return {
			clap::named_flag(m_no_recycle, "n,no-recycle", "skip recycling snapshots"),
			clap::named_flag(m_only_recycle, "r,recycle", "only recycle existing snapshots"),
		};
	}

	auto execute(SubvolumeList& subvolume_list) -> ExitCode final {
		if (!m_only_recycle) {
			auto const results = subvolume_list.take_snapshots();
			if (results.empty() || !all_success(results)) {
				return ExitCode::Failure;
			}
		}

		if (!m_no_recycle) {
			subvolume_list.recycle_snapshots();
		}

		return ExitCode::Success;
	}

	bool m_no_recycle{};
	bool m_only_recycle{};
};

class List : public ICommand {
	[[nodiscard]] auto get_identifier() const -> std::string_view final { return "list"; }
	[[nodiscard]] auto get_help_text() const -> std::string_view final { return "print saved snapshots"; }

	auto execute(SubvolumeList& subvolume_list) -> ExitCode final {
		subvolume_list.print_snapshots(std::cout);
		return ExitCode::Success;
	}
};

class Delete : public ICommand {
	[[nodiscard]] auto get_identifier() const -> std::string_view final { return "delete"; }
	[[nodiscard]] auto get_help_text() const -> std::string_view final { return "delete ALL saved snapshots"; }

	auto execute(SubvolumeList& subvolume_list) -> ExitCode final {
		auto const results = subvolume_list.clear_all_snapshots();
		if (results.empty() || all_success(results)) {
			return ExitCode::Success;
		}
		return ExitCode::Failure;
	}
};
} // namespace

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
		spec.commands.push_back(
			clap::command(subcommand->get_identifier(), subcommand->get_parameter_list(), subcommand->get_help_text()));
	}

	auto const parse_result = clap::parse_main(std::move(spec), argc, argv);
	if (parse_result.should_early_exit()) {
		return static_cast<ExitCode>(parse_result.exit_code());
	}

	m_command_identifier = parse_result.command_identifier();
	return {};
}

auto Runner::execute(SubvolumeList& subvolume_list) -> ExitCode {
	auto const it = std::ranges::find_if(commands, [this](auto const& cmd) {
		return cmd->get_identifier() == m_command_identifier;
	});
	klib_assert(it != commands.end());
	auto& command = *it;
	return command->execute(subvolume_list);
}

auto App::run(int const argc, char const* const* argv) -> int {
	auto runner = Runner{};

	runner.commands.push_back(std::make_unique<TakeSnapshot>());
	runner.commands.push_back(std::make_unique<List>());
	runner.commands.push_back(std::make_unique<Delete>());

	auto options = clap::OptionList{
		clap::named_option(m_custom_config_path, "c,config", "path to custom config"),
	};
	if (auto const exit_code = runner.parse_args(std::move(options), argc, argv)) {
		return static_cast<int>(*exit_code);
	}

	if (!load_config() || !setup_instance() || !load_subvolumes()) {
		return 1;
	}

	return static_cast<int>(runner.execute(m_subvolume_list));
}

auto App::load_config() -> bool {
	auto result = util::load_config(m_custom_config_path);
	if (!result) {
		std::println(std::cerr, "Failed to load config: {}", result.error().message);
		return false;
	}

	m_instance_config = std::move(*result);
	return true;
}

auto App::setup_instance() -> bool {
	auto instance = Instance::create(m_instance_config.storage);
	if (!instance) {
		std::println("Failed to create Instance: {}", instance.error().message);
		return false;
	}

	m_instance.emplace(std::move(*instance));
	return true;
}

auto App::load_subvolumes() -> bool {
	m_subvolume_list = m_instance->load_subvolume_list(std::move(m_instance_config.subvolumes));
	if (!m_subvolume_list.subvolumes.empty()) {
		return true;
	}

	std::println(std::cerr, "No valid subvolumes in loaded configs");
	return false;
}
} // namespace btrsnap::cli
