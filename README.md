# btrsnap

**Barebones tool to take `btrfs` snapshots**

[![Build Status](https://github.com/karnkaul/btrsnap/actions/workflows/ci.yml/badge.svg)](https://github.com/karnkaul/btrsnap/actions/workflows/ci.yml)

## Disclaimer

No warranty, use at your own risk.

This is experimental and hobbyist software, it is strongly recommended to use (much) more robust and popular alternatives like [timeshift](https://github.com/linuxmint/timeshift) and/or [snapper](https://github.com/openSUSE/snapper).

## Requirements

### Runtime

- Linux with at least one `btrfs` filesystem
- GCC 15+ (its associated `libstdc++`)
- `libbtrfsutil`

### Build-time

- CMake 4.3+
- GCC 15+ / Clang 22+
- `libbtrfsutil` and its development header

## Usage

Invoking `btrsnap` takes a snapshot and garbage-collects oldest snapshots across all configured subvolumes. Unless the subvolumes are mounted with permissions for users to create (and delete) snapshots, this will require privileged execution.

Create a configuration file for a subvolume by passing `--generate=/path/to/subvolume`, and save one or more such files to `/etc/btrsnap/`. Snapshots are stored in `/path/to/subvolume/.snapshots` by default (configurable), and limited to `3` by default (also configurable). The snapshots sub-path must also be a subvolume.

Since v0.2, a snapshot **archive** is also supported, located at `/path/to/subvolume/.snapshots/.archive` by default with the same limit as **live** snapshots, and a default period of 7 days. Retired live snapshots will be moved into the archive if the timestamp difference is greater than or equal to the configured period. Remaining retired snapshots (including ones that failed to be archived) are deleted as before, to match the configured limits of both locations.

Pass `--list` to print a list of existing snapshots across all configured subvolumes. Pass `--clear` to delete **ALL** saved snapshots.

### Scheduling

Use `cron` or `systemd` service and timer units.

### Limitations

1. Flat list of snapshots: no daily/weekly/etc buckets
1. Oldest snapshots are indiscriminately deleted to keep the total under the configured limit

## Building

1. Clone repo somewhere, say `./btrsnap`
1. `cd` to `./btrsnap`
1. Run `cmake -S . --preset=default` (or desired preset)
1. Run `cmake --build --preset=Debug` (or desired build config)
1. Output will be `./btrsnap/out/default/cli/Debug/btrsnap`
