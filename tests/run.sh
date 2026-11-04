#!/bin/sh
# Test runner for the Ама compiler.
#
# Usage: ./tests/run.sh
# Or:    make test
#
# Each subdirectory of tests/ that contains a `main` file is a test.
# The directory is treated as an Ама project root. The runner:
#   1. compiles it with ./ama,
#   2. compiles the resulting C with gcc,
#   3. runs the binary and compares stdout with expected.txt.
#
# Tests without expected.txt are skipped (useful as playgrounds).
#
# All intermediate files go to build/tests/ (ignored by git).

set -u

# Move to project root (where the ama binary lives).
cd "$(dirname "$0")/.." || exit 1

if [ ! -x ./ama ]; then
    echo "error: ./ama not found; run 'make' first" >&2
    exit 1
fi

TESTS_DIR=tests
BUILD_DIR=build/tests
mkdir -p "$BUILD_DIR"

passed=0
failed=0
skipped=0
failed_names=""

for test_dir in "$TESTS_DIR"/*/; do
    test_dir=${test_dir%/}
    name=$(basename "$test_dir")

    if [ ! -f "$test_dir/main" ]; then
        skipped=$((skipped + 1))
        continue
    fi

    if [ ! -f "$test_dir/expected.txt" ]; then
        echo "=== $name === (skipped: no expected.txt)"
        skipped=$((skipped + 1))
        continue
    fi

    echo "=== $name ==="

    c_file="$BUILD_DIR/$name.c"
    bin_file="$BUILD_DIR/$name"

    # 1. Ама → C
    if ! ./ama "$test_dir" main "$c_file" > "$BUILD_DIR/$name.ama.log" 2>&1; then
        echo "  FAIL: ama compilation failed"
        sed 's/^/    /' "$BUILD_DIR/$name.ama.log"
        failed=$((failed + 1))
        failed_names="$failed_names $name"
        continue
    fi

    # 2. C → binary
    if ! gcc -O2 "$c_file" -o "$bin_file" 2> "$BUILD_DIR/$name.gcc.log"; then
        echo "  FAIL: gcc compilation failed"
        sed 's/^/    /' "$BUILD_DIR/$name.gcc.log"
        failed=$((failed + 1))
        failed_names="$failed_names $name"
        continue
    fi

    # 3. run and compare stdout
    actual=$("$bin_file")
    expected=$(cat "$test_dir/expected.txt")

    if [ "$actual" = "$expected" ]; then
        echo "  PASS"
        passed=$((passed + 1))
    else
        echo "  FAIL: output mismatch"
        echo "    expected: $(printf '%s' "$expected" | head -c 200)"
        echo "    actual:   $(printf '%s' "$actual" | head -c 200)"
        failed=$((failed + 1))
        failed_names="$failed_names $name"
    fi
done

echo ""
echo "=== Summary ==="
echo "  passed:  $passed"
echo "  failed:  $failed"
if [ "$skipped" -gt 0 ]; then
    echo "  skipped: $skipped"
fi

if [ "$failed" -ne 0 ]; then
    echo "  failed tests:$failed_names"
    exit 1
fi