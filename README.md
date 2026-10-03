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

Invoking `btrsnap` takes a snapshot and recycles oldest snapshots across all configured subvolumes. Unless the subvolumes are mounted with permissions for users to create (and delete) snapshots, this will require privileged execution.

`btrsnap` requires a storage root, defaulting to `/btrsnap`: a directory where all snapshots (live and archived) for all subvolumes will be created and stored. While `btrsnap` will create subdirectories as needed, the storage root is required to already exist. It is recommended to have a separate btrfs subvolume mounted somewhere on the filesystem for this path.

```
├── <storage-root>/
│   ├── snapshots/
│   │   └── <subvolume_name>/
│   │       └── <subvolume_timestamp>
│   │       └── ...
│   │   └── ...
│   └── archive
│       └── ...
```

Since v0.2, a snapshot **archive** is also supported, by default with the same limit as **live** snapshots, and a default period of 7 days. Retired live snapshots will be moved into the archive if the timestamp difference is greater than or equal to the configured period. Remaining retired snapshots (including ones that failed to be archived) are deleted as before, to match the configured limits of both locations.

Pass `--list` to print a list of existing snapshots across all configured subvolumes. Pass `--clear` to delete **ALL** saved snapshots.

### Configuration

`btrsnap` requires a JSON configuration file - `/etc/btrsnap.jsonc` by default - that lists each desired subvolume's name and path.

```json
{
  "subvolumes": [
    {
      "name": "@home",
      "path": "/@home"
    }
  ]
}
```

[JSON schema](schema.json).

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
