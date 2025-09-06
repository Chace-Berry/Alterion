@echo on
REM Build and run lexertest using CMake
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

echo [Building lexertest...]
cmake --build . --target lexertest
if errorlevel 1 (
    echo [ERROR] Build failed!
    pause
    exit /b 1
)

cd ..

REM Run lexertest from build\Debug directory
if exist build\Debug\lexertest.exe (
    echo [Running lexertest.exe...]
    build\Debug\lexertest.exe
    if errorlevel 1 (
        echo [ERROR] Test execution failed!
        pause
        exit /b 1
    )
    echo [Showing lexer_test_output.csv...]
    if exist lexer_test_output.csv (
        more lexer_test_output.csv
    ) else (
        echo [ERROR] lexer_test_output.csv not found!
    )
) else (
    echo [ERROR] build\Debug\lexertest.exe not found!
)
pause
