@echo off
echo Building Assimp static library for MSVC x64...

SET SCRIPT_DIR=%~dp0
SET BUILD_DIR=%SCRIPT_DIR%build\msvc_build
SET OUTPUT_DIR=%SCRIPT_DIR%build\lib

cmake "%SCRIPT_DIR%" ^
    -G "Visual Studio 17 2022" ^
    -A x64 ^
    -B "%BUILD_DIR%" ^
    -DBUILD_SHARED_LIBS=OFF ^
    -DASSIMP_BUILD_TESTS=OFF ^
    -DASSIMP_BUILD_ASSIMP_TOOLS=OFF ^
    -DASSIMP_BUILD_SAMPLES=OFF ^
    -DASSIMP_WARNINGS_AS_ERRORS=OFF ^
    -DASSIMP_INJECT_DEBUG_POSTFIX=OFF

if errorlevel 1 (
    echo CMake configuration failed.
    pause
    exit /b 1
)

cmake --build "%BUILD_DIR%" --config Release

if errorlevel 1 (
    echo Build failed.
    pause
    exit /b 1
)

:: Copy the built lib to the expected location
for /r "%BUILD_DIR%\lib\Release" %%f in (assimp*.lib) do (
    echo Copying %%f to %OUTPUT_DIR%\assimp.lib
    copy /y "%%f" "%OUTPUT_DIR%\assimp.lib"
)

:: Also copy generated config header
if exist "%BUILD_DIR%\include\assimp\config.h" (
    copy /y "%BUILD_DIR%\include\assimp\config.h" "%SCRIPT_DIR%build\include\assimp\config.h"
)

echo.
echo Done! assimp.lib is ready in %OUTPUT_DIR%
pause
