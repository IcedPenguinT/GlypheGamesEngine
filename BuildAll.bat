@ECHO OFF
REM Building everything

ECHO "Building everything..."

PUSHD engine
CALL Build.bat
POPD
IF %ERRORLEVEL% NEQ 0 (ECHO Error:%ERRORLEVEL% && EXIT)

PUSHD testbed
CALL Build.bat
POPD
IF %ERRORLEVEL% NEQ 0 (ECHO Error:%ERRORLEVEL% && EXIT)

ECHO "All assemblies built successfully."