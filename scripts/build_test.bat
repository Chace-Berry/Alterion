@echo off
echo Building Alterion test...

g++ -std=c++17 -I. -o test_complete.exe test_complete.cpp
if %ERRORLEVEL% EQU 0 (
    echo Build successful!
    echo Running test...
    test_complete.exe
) else (
    echo Build failed!
)

pause
