@echo off
REM Run the AST test and output results to results/ast-results.json
cd /d %~dp0\..

echo [Configuring project with CMake...]
if not exist build mkdir build
cd build
cmake ..
if errorlevel 1 (
    echo [ERROR] CMake configuration failed!
    pause
    exit /b 1
)

echo [Building asttest...]
cmake --build . --target asttest
if errorlevel 1 (
    echo [ERROR] Build failed!
    pause
    exit /b 1
)

cd ..
REM
if exist build\Debug\asttest.exe (
    echo [Running asttest.exe...]
    build\Debug\asttest.exe
    if errorlevel 1 (
        echo [ERROR] Test execution failed!
        pause
        exit /b 1
    )
) else (
    echo [ERROR] build\Debug\asttest.exe not found!
)
pause
