#!/usr/bin/env bash
# Battery status plugin for tinyfetch (cross-platform Linux & macOS)
set -euo pipefail

# ANSI colors
ESC=$(printf '\033')
GREEN="${ESC}[01;32m"
YELLOW="${ESC}[01;33m"
RED="${ESC}[01;31m"
BLUE="${ESC}[01;34m"
RESTORE="${ESC}[0m"

capacity=""
status=""
os_type=$(uname -s)

if [ "$os_type" = "Darwin" ]; then
  if command -v pmset >/dev/null 2>&1; then
    batt_out=$(pmset -g batt 2>/dev/null)
    if echo "$batt_out" | grep -q "InternalBattery"; then
      capacity=$(echo "$batt_out" | grep -o '[0-9]\+%' | tr -d '%')
      if echo "$batt_out" | grep -q "discharging"; then
        status="Discharging"
      else
        status="Charging"
      fi
    fi
  fi
else
  # Linux lookup
  for bat in /sys/class/power_supply/BAT*; do
    if [ -d "$bat" ]; then
      capacity=$(cat "$bat/capacity" 2>/dev/null || echo "")
      status=$(cat "$bat/status" 2>/dev/null || echo "")
      break
    fi
  done
fi

# Exit if no battery found
if [ -z "$capacity" ]; then
  exit 0
fi

# Format output
icon="🔋"
color="$GREEN"

if [ "$capacity" -le 20 ]; then
  color="$RED"
elif [ "$capacity" -le 50 ]; then
  color="$YELLOW"
fi

if [ "$status" = "Charging" ]; then
  icon="🔌"
  color="$BLUE"
fi

echo "Battery: ${color}${icon} ${capacity}%${RESTORE} (${status})"
