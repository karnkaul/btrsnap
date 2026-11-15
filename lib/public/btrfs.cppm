export module btrsnap.btrfs;

export import btrsnap.types;

export namespace btrsnap {
class IBtrfs : public klib::Polymorphic {
  public:
	[[nodiscard]] static auto get_default() -> IBtrfs const&;

	[[nodiscard]] virtual auto is_subvolume(klib::CString path) const -> Result<void> = 0;
	[[nodiscard]] virtual auto create_snapshot(klib::CString src, klib::CString dst) const -> Result<void> = 0;
	[[nodiscard]] virtual auto create_readonly_snapshot(klib::CString src, klib::CString dst) const -> Result<void> = 0;
	[[nodiscard]] virtual auto create_subvolume(klib::CString path) const -> Result<void> = 0;
	[[nodiscard]] virtual auto delete_subvolume(klib::CString path) const -> Result<void> = 0;
};
} // namespace btrsnap
