#include "app/app.hpp"
#include "btrsnap/util.hpp"
#include <algorithm>
#include <iostream>
#include <print>

namespace btrsnap::cli {
namespace {
[[nodiscard]] auto all_success(std::span<Result<Snapshot> const> results) {
	return std::ranges::all_of(results, [](auto const& result) { return result.has_value(); });
}

class Root : public IRootCommand {
  public:
	explicit Root(std::string_view& out_config_path) : m_out_config_path(out_config_path) {}

  private:
	[[nodiscard]] auto get_help_text() const -> std::string_view final { return "take and/or recycle snapshots"; }

	auto get_option_list() -> clap::OptionList final {
		return {
			clap::named_option(m_out_config_path, "c,config", "path to custom config"),
			clap::named_flag(m_params.no_recycle, "n,no-recycle", "skip recycling snapshots"),
			clap::named_flag(m_params.only_recycle, "r,recycle", "only recycle existing snapshots"),
		};
	}

	auto execute(SubvolumeList& subvolume_list) -> ExitCode final {
		if (!m_params.only_recycle) {
			auto const results = subvolume_list.take_snapshots();
			if (results.empty() || !all_success(results)) { return ExitCode::Failure; }
		}

		if (!m_params.no_recycle) { subvolume_list.recycle_snapshots(); }

		return ExitCode::Success;
	}

	struct Params {
		bool generate{};
		bool list{};
		bool no_recycle{};
		bool only_recycle{};
		bool clear{};
	};

	std::string_view& m_out_config_path;
	Params m_params{};
};

class List : public ISubcommand {
	[[nodiscard]] auto get_identifier() const -> std::string_view final { return "list"; }
	[[nodiscard]] auto get_help_text() const -> std::string_view final { return "print saved snapshots"; }

	auto execute(SubvolumeList& subvolume_list) -> ExitCode final {
		subvolume_list.print_snapshots(std::cout);
		return ExitCode::Success;
	}
};

class Delete : public ISubcommand {
	[[nodiscard]] auto get_identifier() const -> std::string_view final { return "delete"; }
	[[nodiscard]] auto get_help_text() const -> std::string_view final { return "delete ALL saved snapshots"; }

	auto execute(SubvolumeList& subvolume_list) -> ExitCode final {
		auto const results = subvolume_list.clear_all_snapshots();
		if (results.empty() || all_success(results)) { return ExitCode::Success; }
		return ExitCode::Failure;
	}
};
} // namespace

auto App::run(int const argc, char const* const* argv) -> int {
	auto runner = Runner{};

	runner.root_command = std::make_unique<Root>(m_custom_config_path);
	runner.subcommands.push_back(std::make_unique<List>());
	runner.subcommands.push_back(std::make_unique<Delete>());

	if (auto const exit_code = runner.parse_args(argc, argv)) { return static_cast<int>(*exit_code); }
	if (!load_config() || !setup_instance() || !load_subvolumes()) { return EXIT_FAILURE; }

	return static_cast<int>(runner.execute(m_subvolume_list));
}

auto App::load_config() -> bool {
	auto result = util::load_config(m_custom_config_path);
	if (!result) {
		std::println(stderr, "Failed to load config: {}", result.error().message);
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
	if (!m_subvolume_list.subvolumes.empty()) { return true; }

	std::println(stderr, "No valid subvolumes in loaded configs");
	return false;
}
} // namespace btrsnap::cli
