@echo off
echo Regenerating Visual Studio solution from CMakeLists.txt...
echo.

REM Chemin vers le CMake de Visual Studio
set CMAKE_PATH="C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"

REM Si Visual Studio 2022 Professional ou Enterprise, ajuste le chemin :
REM set CMAKE_PATH="C:\Program Files\Microsoft Visual Studio\2022\Professional\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"

REM Vérifier que CMake existe
if not exist %CMAKE_PATH% (
    echo ERROR: CMake not found at %CMAKE_PATH%
    echo Please adjust the CMAKE_PATH variable in this script.
    pause
    exit /b 1
)

REM Générer la solution
%CMAKE_PATH% -B build ^
      -G "Visual Studio 17 2022" ^
      -A x64 ^
      -DCMAKE_PREFIX_PATH="C:/Qt/6.8.3/msvc2022_64" ^
      -DSDK_ROOT="C:/Dev/SDK"

if %ERRORLEVEL% EQU 0 (
    echo.
    echo ========================================
    echo Solution generated successfu
