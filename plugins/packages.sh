#!/usr/bin/env bash
# Package manager count plugin for tinyfetch (pacman and paru/yay)
set -euo pipefail

# Check if pacman exists
if ! command -v pacman >/dev/null 2>&1; then
  exit 0
fi

# Count native packages
native=$(pacman -Qn 2>/dev/null | wc -l | xargs)

# Count foreign (AUR) packages
foreign=$(pacman -Qm 2>/dev/null | wc -l | xargs)

# Detect AUR helper
helper="pacman"
if command -v paru >/dev/null 2>&1; then
  helper="paru"
elif command -v yay >/dev/null 2>&1; then
  helper="yay"
fi

# Build details
output=""
if [ "$native" -gt 0 ]; then
  output="${native} (pacman)"
fi

if [ "$foreign" -gt 0 ]; then
  if [ -n "$output" ]; then
    output="${output} + "
  fi
  output="${output}${foreign} (${helper})"
fi

if [ -n "$output" ]; then
  echo "Packages: $output"
fi
