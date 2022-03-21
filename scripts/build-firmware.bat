@ECHO OFF

arduino-cli compile -v^
  -b "esp32:esp32:featheresp32"^
  --output-dir ".\target\debug"^
  --build-property "compiler.cpp.extra_flags='-I.'"^
  ".\src\redink"

IF %errorlevel% NEQ 0 EXIT /b %errorlevel%

IF "%1" == "" (
  ECHO No port provided, skippping upload. usage: 'build.bat COM7'
  EXIT 0
)

ECHO Attempting to upload to '%1'
arduino-cli upload -v -p %1 -b "esp32:esp32:featheresp32" --input-dir ".\target\debug"
