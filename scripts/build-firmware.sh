#!/bin/bash

if [[ `which arduino-cli` == "" ]]; then
  echo "missing 'arduino-ci"
  exit 1
fi

arduino-cli compile -v \
  -b esp32:esp32:featheresp32 \
  --output-dir target/debug \
  --build-property "compiler.cpp.extra_flags=-I." \
  src/redink

if [[ "${1}" == "" ]]; then
  echo "Skipping upload. Provide port ('build.sh /dev/ttyACM0')"
  exit 0
fi

arduino-cli upload -v \
  -b esp32:esp32:featheresp32 \
  -p "${1}" \
  --input-dir target/debug \
