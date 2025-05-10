#!/bin/bash

TEST_DIR="tests"

for TEST_INPUT in "${TEST_DIR}"/*.in; do
  if [[ "${TEST_INPUT}" =~ test_([0-9]+)_([0-9]+)_([0-9]+)_([0-9]+)\.in$ ]]; then
    MODE=${BASH_REMATCH[1]}
    LEVEL=${BASH_REMATCH[2]}
    VERSION=${BASH_REMATCH[3]}
    LENGTH=${BASH_REMATCH[4]}

    for MASK in $(seq 0 7); do
      TEST_OUTPUT="${TEST_INPUT%.in}_${MASK}.out"
      TEST_EXPECTED="${TEST_INPUT%.in}_${MASK}.exp"

      ./bin/qr -o ASCII -l "${LEVEL}" -v "${VERSION}" -m "${MASK}" -f "${TEST_INPUT}" > "${TEST_OUTPUT}"

      if ! cmp -s "${TEST_EXPECTED}" "${TEST_OUTPUT}"; then
        echo "❌ '${TEST_OUTPUT}' doesn't match '${TEST_EXPECTED}'"
      fi
    done
  else
    echo "Failed to match ${TEST_INPUT}"
  fi
done
