#include "btrsnap/build_version.hpp"
#include "btrsnap/instance.hpp"
#include "btrsnap/util.hpp"
#include "clap/parser.hpp"
#include "klib/debug/assert.hpp"
#include <iostream>
#include <optional>
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

		if (!load_config()) { return EXIT_FAILURE; }
		KLIB_ASSERT(!m_instance_config.subvolumes.empty());

		if (!setup_instance() || !load_subvolumes()) { return EXIT_FAILURE; }
		KLIB_ASSERT(!m_subvolume_list.subvolumes.empty());

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
					clap::named_option(m_params.custom_config_path, "c,config", "path to custom config"),
					clap::named_flag(m_params.list, "l,list", "list snapshots"),
					clap::named_flag(m_params.no_recycle, "n,no-recycle", "skip recycling snapshots"),
					clap::named_flag(m_params.only_recycle, "r,recycle", "only recycle existing snapshots"),
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

	[[nodiscard]] auto load_config() -> bool {
		auto result = util::load_config(m_params.custom_config_path);
		if (!result) {
			std::println(stderr, "Failed to load config: {}", result.error().message);
			return false;
		}

		m_instance_config = std::move(*result);
		return true;
	}

	[[nodiscard]] auto setup_instance() -> bool {
		auto instance = Instance::create(m_instance_config.storage);
		if (!instance) {
			std::println("Failed to create Instance: {}", instance.error().message);
			return false;
		}

		m_instance.emplace(std::move(*instance));
		return true;
	}

	[[nodiscard]] auto load_subvolumes() -> bool {
		m_subvolume_list = m_instance->load_subvolume_list(std::move(m_instance_config.subvolumes));
		if (!m_subvolume_list.subvolumes.empty()) { return true; }

		std::println(stderr, "No valid subvolumes in loaded configs");
		return false;
	}

	[[nodiscard]] auto print_snapshots() const -> int {
		m_subvolume_list.print_snapshots(std::cout);
		return EXIT_SUCCESS;
	}

	[[nodiscard]] auto clear_snapshots() -> int {
		auto const results = m_subvolume_list.clear_all_snapshots();
		if (results.empty() || all_success(results)) { return EXIT_SUCCESS; }
		return EXIT_FAILURE;
	}

	[[nodiscard]] auto take_snapshots() -> int {
		if (!m_params.only_recycle) {
			auto const results = m_subvolume_list.take_snapshots();
			if (results.empty() || !all_success(results)) { return EXIT_FAILURE; }
		}

		if (!m_params.no_recycle) { m_subvolume_list.recycle_snapshots(); }

		return EXIT_SUCCESS;
	}

	struct Params {
		std::string_view custom_config_path{};
		bool generate{};
		bool list{};
		bool no_recycle{};
		bool only_recycle{};
		bool clear{};
	};

	Params m_params{};

	InstanceConfig m_instance_config{};
	std::optional<Instance> m_instance{};
	SubvolumeList m_subvolume_list{};
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
