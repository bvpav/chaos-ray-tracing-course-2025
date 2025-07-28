@echo off
setlocal

rem Configure CMake and build directories
set "CMAKE=cmake"
set "BUILD_DIR=build"
set "BUILD_STANDALONE_DIR=%BUILD_DIR%\standalone"
set "BUILD_PYTHON_DIR=%BUILD_DIR%\python"
set "BUILD_BLENDER_DIR=%BUILD_DIR%\blender"

rem Dispatch on the first argument
if "%~1"=="" (
  echo Usage: make.bat [standalone^|python^|blender^|clean]
  exit /b 1
)
if /I "%~1"=="standalone" goto :STANDALONE
if /I "%~1"=="python"     goto :PYTHON
if /I "%~1"=="blender"    goto :BLENDER
if /I "%~1"=="clean"      goto :CLEAN

echo Unknown target "%~1"
echo Usage: make.bat [standalone^|python^|blender^|clean]
exit /b 1

:STANDALONE
mkdir "%BUILD_STANDALONE_DIR%" 2>nul
"%CMAKE%" -S . -B "%BUILD_STANDALONE_DIR%" ^
  -DBUILD_STANDALONE=ON ^
  -DBUILD_PYTHON=OFF ^
  -DBUILD_BLENDER_EXTENSION=OFF
if errorlevel 1 exit /b %errorlevel%
"%CMAKE%" --build "%BUILD_STANDALONE_DIR%"
if errorlevel 1 exit /b %errorlevel%

echo.
echo ^> Build finished successfully
echo ^> Run with .\build\standalone\crt_renderer.exe ^<scene_file^> ^<output_file^>
echo.
exit /b 0

:PYTHON
mkdir "%BUILD_PYTHON_DIR%" 2>nul
"%CMAKE%" -S . -B "%BUILD_PYTHON_DIR%" ^
  -DBUILD_STANDALONE=OFF ^
  -DBUILD_PYTHON=ON ^
  -DBUILD_BLENDER_EXTENSION=OFF
if errorlevel 1 exit /b %errorlevel%
"%CMAKE%" --build "%BUILD_PYTHON_DIR%" --target _crt
if errorlevel 1 exit /b %errorlevel%

echo.
echo ^> Build finished successfully
echo ^> You can import the _crt module from .\build\python
echo.
exit /b 0

:BLENDER
mkdir "%BUILD_BLENDER_DIR%" 2>nul
"%CMAKE%" -S . -B "%BUILD_BLENDER_DIR%" ^
  -DBUILD_STANDALONE=OFF ^
  -DBUILD_PYTHON=ON ^
  -DBUILD_BLENDER_EXTENSION=ON
if errorlevel 1 exit /b %errorlevel%
"%CMAKE%" --build "%BUILD_BLENDER_DIR%" --target blender_extension
if errorlevel 1 exit /b %errorlevel%

echo.
echo ^> Build finished successfully
echo ^> ZIP archive is in .\build\blender\crt_blender_extension.zip
echo ^> Install it via Blender's User Preferences → Add-ons → Install…
echo.
exit /b 0

:CLEAN
rmdir /S /Q "%BUILD_DIR%"
echo.
echo ^> Clean complete, removed .\build directory
echo.
exit /b 0
