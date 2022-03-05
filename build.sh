#!/bin/bash

if [[ `which arduino-cli` == "" ]]; then
  echo "missing 'arduino-ci"
  exit 1
fi

arduino-cli compile -v \
  -b adafruit:samd:adafruit_itsybitsy_m4 \
  --output-dir target/debug \
  --build-property "compiler.cpp.extra_flags=-I." \
  src/net-display

if [[ "${1}" == "" ]]; then
  echo "Skipping upload. Provide port ('build.sh /dev/ttyACM0')"
  exit 0
fi

arduino-cli upload -v \
  -b adafruit:samd:adafruit_itsybitsy_m4 \
  -p "${1}" \
  --input-dir target/debug \
