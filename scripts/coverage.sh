#!/bin/bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"

BUILD_DIR="${1:-build-cov}"

cmake -B "${BUILD_DIR}" "${ROOT_DIR}/native" -GNinja \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_CXX_FLAGS="--coverage -fprofile-arcs -ftest-coverage" \
    -DCMAKE_C_FLAGS="--coverage -fprofile-arcs -ftest-coverage" \
    -DCMAKE_EXE_LINKER_FLAGS="--coverage" \
    -DBUILD_TESTING=ON

cmake --build "${BUILD_DIR}" --parallel
ctest --test-dir "${BUILD_DIR}" --output-on-failure -j4

# Generate lcov coverage report.
lcov --capture \
    --directory "${BUILD_DIR}" \
    --output-file "${BUILD_DIR}/coverage_raw.info" \
    --ignore-errors mismatch

# Filter out third-party and test code.
lcov --remove "${BUILD_DIR}/coverage_raw.info" \
    '*/third_party/*' \
    '*/test/*' \
    '/usr/*' \
    --output-file "${BUILD_DIR}/coverage.info" \
    --ignore-errors unused

echo "Coverage report: ${BUILD_DIR}/coverage.info"
lcov --summary "${BUILD_DIR}/coverage.info" || true
