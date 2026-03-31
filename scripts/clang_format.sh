#!/bin/bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"

MODE="${1:-check}"  # "check" or "fix"

CLANG_FORMAT="clang-format-19"
if ! command -v "${CLANG_FORMAT}" &>/dev/null; then
    CLANG_FORMAT="clang-format"
fi

echo "Using: $(command -v "${CLANG_FORMAT}")"

# Find all project source files (not third-party, not vendored Dart headers).
FILES=$(find "${ROOT_DIR}/native/src" "${ROOT_DIR}/native/include" \
    \( -name '*.cpp' -o -name '*.h' \) \
    ! -path '*/third_party/*' \
    ! -path '*/internal/*' \
    ! -name 'dart_api.h' \
    ! -name 'dart_api_dl.h' \
    ! -name 'dart_native_api.h' \
    ! -name 'dart_version.h' \
    -print | sort)

if [[ "${MODE}" == "check" ]]; then
    echo "Checking format..."
    FAILED=0
    for f in ${FILES}; do
        if ! "${CLANG_FORMAT}" --dry-run --Werror "${f}" 2>/dev/null; then
            FAILED=1
        fi
    done
    if [[ ${FAILED} -eq 1 ]]; then
        echo "Format check failed. Run: ./scripts/clang_format.sh fix"
        exit 1
    fi
    echo "Format check passed."
elif [[ "${MODE}" == "fix" ]]; then
    echo "Formatting..."
    for f in ${FILES}; do
        "${CLANG_FORMAT}" -i "${f}"
    done
    echo "Done."
else
    echo "Usage: $0 [check|fix]"
    exit 1
fi
