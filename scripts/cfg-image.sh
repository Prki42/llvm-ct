#!/usr/bin/env bash

set -euo pipefail

usage() {
	cat <<'EOF'
Usage: cfg-image.sh [OPTIONS] <file.ll>

Generate a CFG image from an LLVM IR file.

Options:
  -o <path>     Save image to <path> instead of opening in viewer
  -f <format>   Output format: svg (default) or png
  -h            Show this help message
EOF
	exit "${1:-0}"
}

FORMAT=svg
OUTPUT=""

while getopts "o:f:h" opt; do
	case "$opt" in
	o) OUTPUT="$OPTARG" ;;
	f) FORMAT="$OPTARG" ;;
	h) usage 0 ;;
	*) usage 1 ;;
	esac
done
shift $((OPTIND - 1))

if [[ $# -ne 1 ]]; then
	echo "Error: expected exactly one .ll file" >&2
	usage 1
fi

INPUT="$1"

if [[ ! -f "$INPUT" ]]; then
	echo "Error: file not found: $INPUT" >&2
	exit 1
fi

if [[ "$FORMAT" != "svg" && "$FORMAT" != "png" ]]; then
	echo "Error: unsupported format '$FORMAT', use svg or png" >&2
	exit 1
fi

TMPDIR="$(mktemp -d)"
trap 'rm -rf "$TMPDIR"' EXIT

opt --passes=dot-cfg -disable-output "$INPUT" 2>/dev/null
mv .*.dot "$TMPDIR/" 2>/dev/null

COMBINED="$TMPDIR/combined.dot"
echo "digraph CFG {" >"$COMBINED"
for dotfile in "$TMPDIR"/.*.dot; do
	[[ -f "$dotfile" ]] || continue
	sed '1d;$d' "$dotfile" >>"$COMBINED"
done
echo "}" >>"$COMBINED"

if [[ -n "$OUTPUT" ]]; then
	dot -T"$FORMAT" "$COMBINED" -o "$OUTPUT" 2>/dev/null
	echo "Saved to $OUTPUT"
else
	TMPOUT="$TMPDIR/cfg.$FORMAT"
	dot -T"$FORMAT" "$COMBINED" -o "$TMPOUT" 2>/dev/null
	trap - EXIT
	(
		xdg-open "$TMPOUT" &>/dev/null
		rm -rf "$TMPDIR"
	) &
	disown
fi
