#include "btrsnap/build_version.hpp"
#include "btrsnap/instance.hpp"
#include "clap/parser.hpp"
#include "klib/debug/assert.hpp"
#include <iostream>
#include <print>

namespace btrsnap::cli {
namespace {
[[nodiscard]] auto all_success(std::span<Result<Snapshot> const> results) {
	return std::ranges::all_of(results, [](auto const& result) { return result.has_value(); });
}

class App {
  public:
	auto run(int const argc, char const* const* argv) -> int {
		auto const parse_result = parse_args(argc, argv);
		if (parse_result.should_early_exit()) { return parse_result.return_code(); }

		if (m_params.generate) {
			auto const config = Config{.subvolume = std::string{m_params.generate_for_subvolume}};
			config.print();
			return EXIT_SUCCESS;
		}

		if (!fill_configs()) { return EXIT_FAILURE; }
		KLIB_ASSERT(!m_configs.empty());

		if (!setup_instance()) { return EXIT_FAILURE; }
		KLIB_ASSERT(!m_instance.get_loaded_subvolumes().empty());

		if (m_params.list) { return print_snapshots(); }
		if (m_params.clear) { return clear_snapshots(); }
		return take_snapshots();
	}

  private:
	[[nodiscard]] auto parse_args(int const argc, char const* const* argv) -> clap::Result {
		static auto const version_str = std::format("{}", build_version_v);
		auto spec = clap::spec::Parameters{
			.parameters =
				{
					clap::named_option(m_params.generate_for_subvolume, "g,generate", "generate config for SUBVOLUME", &m_params.generate),
					clap::named_option(m_params.custom_config_path, "c,config", "path to custom config"),
					clap::named_flag(m_params.list, "l,list", "list snapshots"),
					clap::named_flag(m_params.no_trim, "n,no-trim", "skip trimming snapshots"),
					clap::named_flag(m_params.only_trim, "t,trim", "only trim existing snapshots"),
					clap::named_flag(m_params.clear, "clear", "clear ALL saved snapshots"),
				},
			.program =
				clap::Program{
					.name = "btrsnap",
					.version = version_str,
				},
		};
		auto parser = clap::Parser{std::move(spec)};
		return parser.parse_main(argc, argv);
	}

	[[nodiscard]] auto fill_configs() -> bool {
		if (!m_params.custom_config_path.empty()) {
			auto custom_config = Config::from_file(m_params.custom_config_path);
			if (!custom_config) {
				std::println(stderr, "Failed to load config from: {}", m_params.custom_config_path);
				return false;
			}

			m_configs.push_back(std::move(*custom_config));
			return true;
		}

		m_configs = Config::from_directory();
		if (!m_configs.empty()) { return true; }

		std::println(stderr, "No valid configs found in: {}", Config::directory_v);
		return false;
	}

	[[nodiscard]] auto setup_instance() -> bool {
		auto loaded = 0;
		for (auto const& config : m_configs) {
			if (m_instance.load_subvolume(config)) { ++loaded; }
		}
		if (loaded > 0) { return true; }

		std::println(stderr, "No valid subvolumes in loaded configs");
		return false;
	}

	[[nodiscard]] auto print_snapshots() const -> int {
		m_instance.print_snapshots(std::cout);
		return EXIT_SUCCESS;
	}

	[[nodiscard]] auto clear_snapshots() -> int {
		auto const results = m_instance.clear_snapshots();
		if (results.empty() || all_success(results)) { return EXIT_SUCCESS; }
		return EXIT_FAILURE;
	}

	[[nodiscard]] auto take_snapshots() -> int {
		if (!m_params.only_trim) {
			auto const results = m_instance.take_snapshots();
			if (results.empty() || !all_success(results)) { return EXIT_FAILURE; }
		}

		if (!m_params.no_trim) { m_instance.trim_snapshots(); }

		return EXIT_SUCCESS;
	}

	struct Params {
		std::string_view generate_for_subvolume{};
		std::string custom_config_path{};
		bool generate{};
		bool list{};
		bool no_trim{};
		bool only_trim{};
		bool clear{};
	};

	Params m_params{};

	std::vector<Config> m_configs{};
	Instance m_instance{};
};
} // namespace
} // namespace btrsnap::cli

int main(int argc, char** argv) {
	try {
		return btrsnap::cli::App{}.run(argc, argv);
	} catch (std::exception const& e) {
		std::println(stderr, "PANIC: {}", e.what());
		return EXIT_FAILURE;
	} catch (...) {
		std::println(stderr, "PANIC!");
		return EXIT_FAILURE;
	}
}
