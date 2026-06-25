#!/usr/bin/env bash
# Media status plugin for tinyfetch (queries playerctl if available)
set -euo pipefail

# Check if playerctl exists
if ! command -v playerctl >/dev/null 2>&1; then
  exit 0
fi

# Get player status
status=$(playerctl status 2>/dev/null || echo "")

if [ "$status" = "Playing" ] || [ "$status" = "Paused" ]; then
  artist=$(playerctl metadata artist 2>/dev/null || echo "")
  title=$(playerctl metadata title 2>/dev/null || echo "")
  
  # Clean up empty values
  artist=$(echo "$artist" | xargs)
  title=$(echo "$title" | xargs)
  
  # Build description
  track=""
  if [ -n "$artist" ] && [ -n "$title" ]; then
    track="$artist - $title"
  elif [ -n "$title" ]; then
    track="$title"
  fi
  
  if [ -n "$track" ]; then
    ESC=$(printf '\033')
    GREEN="${ESC}[01;32m"
    YELLOW="${ESC}[01;33m"
    RESTORE="${ESC}[0m"
    
    if [ "$status" = "Playing" ]; then
      echo "Music: ${GREEN} ${RESTORE} $track"
    else
      echo "Music: ${YELLOW}󰎆 ${RESTORE} $track (Paused)"
    fi
  fi
fi
