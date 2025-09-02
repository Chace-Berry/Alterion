@echo off
REM Alterion Language Lexer and Parser Build Script for Windows

echo Building Alterion Language Lexer and Parser...

REM Create build directory if it doesn't exist
if not exist build mkdir build

REM Compile the complete test with all dependencies
g++ -std=c++17 -I"include" -O2 -Wall -Wextra tests/complete_test.cpp -o build/alterion_test.exe

if %ERRORLEVEL% EQU 0 (
    echo Build successful!
    echo Running tests...
    echo ====================
    build\alterion_test.exe
) else (
    echo Build failed!
    exit /b 1
)
