#!/bin/bash

# Simple test script for JavaScript SWIG bindings.
# This script runs an end-to-end JavaScript runtime test that creates
# a kernel and an agent through SWIG bindings.

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

if [ -n "${SOAR_BUILD_DIR}" ]; then
    BUILD_DIR="${SOAR_BUILD_DIR}"
else
    for build_type in Debug Release; do
        candidate="${SCRIPT_DIR}/../build/${build_type}"
        if [ -d "${candidate}" ]; then
            BUILD_DIR="${candidate}"
            break
        fi
    done

    if [ -z "${BUILD_DIR}" ]; then
        BUILD_DIR="${SCRIPT_DIR}/../build/Debug"
    fi
fi

echo "Testing JavaScript SWIG bindings..."
echo "Build directory: ${BUILD_DIR}"

export SOAR_BUILD_DIR="${BUILD_DIR}"
JS_TEST_FILE="${SCRIPT_DIR}/SimpleJavaScriptSMLTest.js"

JS_NODE_CANDIDATES=(
    "${BUILD_DIR}/Core/ClientSMLSWIG/javascript/JavaScript_sml_ClientInterface.node"
    "${BUILD_DIR}/Core/ClientSMLSWIG/JavaScript_sml_ClientInterface.node"
    "${SCRIPT_DIR}/../Core/ClientSMLSWIG/JS/JavaScript_sml_ClientInterface.node"
)

if [ ! -f "${JS_TEST_FILE}" ]; then
    echo "[ERROR] JavaScript test file not found: ${JS_TEST_FILE}"
    exit 1
fi

if ! command -v node >/dev/null 2>&1; then
    echo "[ERROR] node executable not found"
    exit 1
fi

HAS_JS_NODE=0
for candidate in "${JS_NODE_CANDIDATES[@]}"; do
    if [ -f "${candidate}" ]; then
        HAS_JS_NODE=1
        break
    fi
done

if [ "${HAS_JS_NODE}" -eq 0 ]; then
    echo "[ERROR] JavaScript SWIG native addon not found."
    echo "[ERROR] Checked:"
    for candidate in "${JS_NODE_CANDIDATES[@]}"; do
        echo "  - ${candidate}"
    done
    exit 1
fi

echo "Running JavaScript SML test: ${JS_TEST_FILE}"
node "${JS_TEST_FILE}"

echo "SUCCESS: JavaScript SWIG bindings test passed!"
