@echo off
setlocal EnableDelayedExpansion

set "ROOT=%~dp0"
set "BUILD_DIR=%ROOT%build-libretro-win"
set "CMAKE=C:\msys64\ucrt64\bin\cmake.exe"
set "RETROARCH_DIR=D:\Emulation\Emulators\RetroArch"
set "CORE_DST=%RETROARCH_DIR%\cores\cemu_libretro.dll"

if not exist "%CMAKE%" (
  echo ERROR: CMake was not found at "%CMAKE%".
  exit /b 1
)

if not exist "%BUILD_DIR%\CMakeCache.txt" (
  echo Configuring libretro build...
  "%CMAKE%" -S "%ROOT%" -B "%BUILD_DIR%" -G "Visual Studio 17 2022" -A x64 -DENABLE_LIBRETRO=ON -DENABLE_CUBEB=ON -DCMAKE_BUILD_TYPE=Release
  if errorlevel 1 exit /b 1
)

echo Building CemuLibretro...
"%CMAKE%" --build "%BUILD_DIR%" --config Release --target CemuLibretro --parallel 4
if errorlevel 1 exit /b 1

echo.
echo Built DLL:
echo   "%ROOT%bin\cemu_libretro.dll"

if /I "%~1"=="install" (
  if not exist "%RETROARCH_DIR%\cores" (
    echo ERROR: RetroArch cores folder was not found at "%RETROARCH_DIR%\cores".
    exit /b 1
  )

  for /f %%a in ('powershell -NoProfile -Command "Get-Date -Format yyyyMMdd-HHmmss"') do set "STAMP=%%a"

  if exist "%CORE_DST%" copy /Y "%CORE_DST%" "%CORE_DST%.bak-!STAMP!" >nul
  copy /Y "%ROOT%bin\cemu_libretro.dll" "%CORE_DST%" >nul
  if errorlevel 1 exit /b 1

  echo Installed to:
  echo   "%CORE_DST%"
)

endlocal
