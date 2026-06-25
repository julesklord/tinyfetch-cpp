#!/usr/bin/env bash
# Docker containers count plugin for tinyfetch
set -euo pipefail

# Check if docker command exists
if ! command -v docker >/dev/null 2>&1; then
  exit 0
fi

# Check if docker daemon is running
if ! docker info >/dev/null 2>&1; then
  exit 0
fi

# Get running and total containers count
running=$(docker ps -q 2>/dev/null | wc -l | xargs)
total=$(docker ps -a -q 2>/dev/null | wc -l | xargs)

# ANSI colors
ESC=$(printf '\033')
BLUE="${ESC}[01;34m"
RESTORE="${ESC}[0m"

if [ "$total" -gt 0 ]; then
  echo "Docker: ${BLUE} ${RESTORE} $running running ($total total)"
else
  echo "Docker: ${BLUE} ${RESTORE} idle (0 containers)"
fi
