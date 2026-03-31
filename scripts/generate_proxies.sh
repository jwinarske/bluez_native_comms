#!/bin/bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"

# Locate sdbus-c++-xml2cpp — check known build-tree paths first, then PATH.
_find_xml2cpp() {
    local candidates=(
        "${ROOT_DIR}/build/tools/sdbus-c++-xml2cpp"
        "${ROOT_DIR}/build-codegen/tools/sdbus-c++-xml2cpp"
        "${ROOT_DIR}/cmake-build-debug/tools/sdbus-c++-xml2cpp"
        "${ROOT_DIR}/cmake-build-release/tools/sdbus-c++-xml2cpp"
    )
    for candidate in "${candidates[@]}"; do
        if [[ -x "${candidate}" ]]; then
            echo "${candidate}"
            return 0
        fi
    done
    if command -v sdbus-c++-xml2cpp &>/dev/null; then
        command -v sdbus-c++-xml2cpp
        return 0
    fi
    echo "ERROR: sdbus-c++-xml2cpp not found." \
         "Build the project first or install sdbus-c++." >&2
    return 1
}

XML2CPP="$(_find_xml2cpp)"
echo "Using: ${XML2CPP}"

# Run from repo root so include guards use relative paths.
cd "${ROOT_DIR}"
mkdir -p native/generated

${XML2CPP} --verbose --proxy=native/generated/adapter1_proxy.h \
    interfaces/org.bluez.Adapter1.xml

${XML2CPP} --verbose --proxy=native/generated/device1_proxy.h \
    interfaces/org.bluez.Device1.xml

${XML2CPP} --verbose --proxy=native/generated/gatt_characteristic1_proxy.h \
    interfaces/org.bluez.GattCharacteristic1.xml

${XML2CPP} --verbose --proxy=native/generated/gatt_service1_proxy.h \
    interfaces/org.bluez.GattService1.xml

${XML2CPP} --verbose --proxy=native/generated/gatt_descriptor1_proxy.h \
    interfaces/org.bluez.GattDescriptor1.xml

${XML2CPP} --verbose --proxy=native/generated/object_manager_proxy.h \
    interfaces/org.freedesktop.DBus.ObjectManager.xml

echo "Proxy generation complete."
