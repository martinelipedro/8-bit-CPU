#!/usr/bin/env bash
set -euo pipefail

# Sequential renamer for media files.
# Usage: ./rename_media.sh [prefix]
# Default prefix: media
# Example output: media_0001.jpg, media_0002.png

prefix="${1:-media}"

# Collect files (ignore directories and this script itself)
mapfile -d '' files < <(find . -maxdepth 1 -type f ! -name 'rename_media.sh' -print0 | sort -z)

if [ ${#files[@]} -eq 0 ]; then
  echo "No files to rename in media/"
  exit 0
fi

# Determine zero padding based on count (minimum 4 digits)
count=${#files[@]}
pad=4
if [ $count -ge 10000 ]; then
  pad=${#count}
fi

i=1
for path in "${files[@]}"; do
  ext="${path##*.}"
  if [[ "$path" == *"."* ]]; then
    new=$(printf "%s_%0${pad}d.%s" "$prefix" "$i" "$ext")
  else
    new=$(printf "%s_%0${pad}d" "$prefix" "$i")
  fi
  if [ -e "$new" ]; then
    echo "Target exists: $new (aborting)"
    exit 1
  fi
  mv -- "$path" "$new"
  echo "$path -> $new"
  i=$((i+1))
done
