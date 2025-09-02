@echo off
echo Building Alterion lexer and parser...

:: Create build directory
if not exist build mkdir build

:: Compile the project
echo Compiling...
cl /I include /std:c++17 /EHsc ^
    lexer/lexer_complete.cpp ^
    parser/parser.cpp ^
    test_main.cpp ^
    /Fe:build/alterion_test.exe

if %errorlevel% == 0 (
    echo Build successful!
    echo Executable created: build/alterion_test.exe
) else (
    echo Build failed!
    exit /b 1
)
