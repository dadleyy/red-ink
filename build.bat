@ECHO OFF

IF "%1" == "" (
  ECHO "Please provide port ('build.bat COM7')"
  EXIT 1
)

arduino-cli compile -v -b "adafruit:samd:adafruit_itsybitsy_m4" --output-dir ".\target\debug" ".\src\net-display"
arduino-cli upload -v -p %1 -b "adafruit:samd:adafruit_itsybitsy_m4" --input-dir ".\target\debug"
