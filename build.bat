@ECHO OFF

arduino-cli compile -v^
  -b "adafruit:samd:adafruit_itsybitsy_m4"^
  --output-dir ".\target\debug"^
  --build-property "compiler.cpp.extra_flags='-I.'"^
  ".\src\net-display"

IF %errorlevel% NEQ 0 EXIT /b %errorlevel%

IF "%1" == "" (
  ECHO No port provided, skippping upload. usage: 'build.bat COM7'
  EXIT 0
)

ECHO Attempting to upload to '%1'
arduino-cli upload -v -p %1 -b "adafruit:samd:adafruit_itsybitsy_m4" --input-dir ".\target\debug"
