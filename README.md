# tinyfetch-cpp

> High-performance compiled C++17 port of **tinyfetch** — a minimal fastfetch-style status tool showing system info in a beautiful, compact layout.

## Overview

**tinyfetch-cpp** is a dependency-free, high-performance C++ implementation of the tiny status CLI. It queries resource metrics directly from native Linux directories (`/proc/uptime`, `/proc/meminfo`, `/proc/cpuinfo`) and formats them side-by-side with an OS logo card in an auto-scaled, responsive multi-column layout.

## Features

- **High Performance**: Compiled native C++17 binary with zero runtime dependencies.
- **Dynamic Terminal Scaling**: Proportional column resizing and automatic responsive component disabling based on active terminal physical columns (`tput cols`).
- **ANSI Truncation & UTF-8 Decode**: Accurate visual length calculations handling multi-byte Unicode characters, emojis, and styling escape sequences.
- **Serialization support**: Format diagnostic output directly into `json`, `xml`, or `txt` representations.
- **Extensibility**: Automatically executes and displays output from custom status elements (simple plugins in `./plugins` and extended plugins in `./plugins/extended`).

## Installation & Build

### Requirements
- GCC 8+ or Clang 7+ (supporting C++17 standard)
- GNU Make

### Compile from Source
```bash
git clone https://github.com/julesklord/tinyfetch-cpp.git
cd tinyfetch-cpp
make build
```

### Install System-Wide
```bash
sudo make install
```

## Command Usage

```bash
tinyfetch [--no-ascii] [--minimal] [--noframe] [--output=json|xml|txt]
```

| Flag | Description |
| :--- | :--- |
| `-h`, `--help` | Display usage instructions and exit. |
| `--no-ascii` | Omit the host OS logo graphics. |
| `--minimal` | Hide extended plugins, printing only the center info card. |
| `--noframe` | Remove boxed borders, separating layouts with spaces. |
| `--output=FMT` | Serialize system specifications into `json`, `xml`, or `txt` data formats. |

## File Structure

```
├── Makefile            # C++ compiler orchestration
├── LICENSE             # MIT License
├── README.md           # Documentation
├── src/                # C++ source code
│   ├── main.cpp        # Command entry & output orchestrator
│   ├── sysinfo.hpp     # System specs module headers
│   ├── sysinfo.cpp     # Linux /proc file parsing implementations
│   ├── render.hpp      # Styling and ANSI formatting headers
│   ├── render.cpp      # UTF-8 decoding & ANSI length algorithms
│   ├── export.hpp      # Output serialization module headers
│   └── export.cpp      # JSON, XML, TXT output formatting implementations
├── ascii/              # OS Logo database text files
└── plugins/            # Execution hooks directory
```

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
