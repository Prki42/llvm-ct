#!/usr/bin/env bash

set -euo pipefail

usage() {
	cat <<'EOF'
Usage: test-transform.sh [OPTIONS] <file.c> [-- args...]

Compiles a C file with and without the ct-branch pass,
runs both with the given args, and diffs the output.

Options:
  -e          Export .ll files next to the .c file
  -E <dir>    Export .ll files to <dir>
  -m          Run sroa,mem2reg before the ct passes
  -h          Show this help message

EOF
	exit "${1:-0}"
}

EXPORT_DIR=""
MEM2REG=false

while getopts "eE:mh" opt; do
	case "$opt" in
	e) EXPORT_DIR="__default__" ;;
	E) EXPORT_DIR="$OPTARG" ;;
	m) MEM2REG=true ;;
	h) usage 0 ;;
	*) usage 1 ;;
	esac
done
shift $((OPTIND - 1))

if [[ $# -lt 1 ]]; then
	usage 1
fi

INPUT="$1"
shift

if [[ "$EXPORT_DIR" == "__default__" ]]; then
	EXPORT_DIR="$(dirname "$INPUT")"
fi

ARGS=()
if [[ "${1:-}" == "--" ]]; then
	shift
	ARGS=("$@")
fi

PLUGIN="build/CTPass.so"
if [[ ! -f "$PLUGIN" ]]; then
	echo "Error: $PLUGIN not found, run cmake --build build first" >&2
	exit 1
fi

TMPDIR="$(mktemp -d)"
trap 'rm -rf "$TMPDIR"' EXIT

# generate clean IR
clang -S -emit-llvm -fno-discard-value-names -O0 -Xclang -disable-O0-optnone -w "$INPUT" -o "$TMPDIR/clean.ll"
if [[ "$MEM2REG" == true ]]; then
	opt --passes=sroa,mem2reg -S "$TMPDIR/clean.ll" -o "$TMPDIR/clean.ll"
fi

# apply ct-branch pass
opt --load-pass-plugin="$PLUGIN" \
	--passes=mergereturn,structurizecfg,ct-branch,ct-data,simplifycfg \
	-S "$TMPDIR/clean.ll" -o "$TMPDIR/transformed.ll"

# export IR if requested
if [[ -n "$EXPORT_DIR" ]]; then
	mkdir -p "$EXPORT_DIR"
	cp "$TMPDIR/clean.ll" "$EXPORT_DIR/clean.ll"
	cp "$TMPDIR/transformed.ll" "$EXPORT_DIR/transformed.ll"
	echo "Exported IR to $EXPORT_DIR/"
fi

# compile both
clang -w "$TMPDIR/clean.ll" -o "$TMPDIR/original"
clang -w "$TMPDIR/transformed.ll" -o "$TMPDIR/transformed"

# run and compare
ORIG_OUT=$("$TMPDIR/original" "${ARGS[@]}" 2>&1) || true
TRANS_OUT=$("$TMPDIR/transformed" "${ARGS[@]}" 2>&1) || true

if [[ "$ORIG_OUT" == "$TRANS_OUT" ]]; then
	echo "PASS - output matches"
	echo "$ORIG_OUT"
else
	echo "FAIL - output differs"
	diff <(echo "$ORIG_OUT") <(echo "$TRANS_OUT")
	exit 1
fi
