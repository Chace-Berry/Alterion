@echo off
REM Build script for Alterion full pipeline test
setlocal
set INCLUDE_DIR=include
set SRC_DIR=src
set TEST_DIR=tests
set OUT_DIR=build
if not exist %OUT_DIR% mkdir %OUT_DIR%
g++ -std=c++17 -I%INCLUDE_DIR% -I. -O2 -Wall -Wextra ^
    %TEST_DIR%\full_pipeline_test.cpp ^
    src/ast_implementation.cpp ^
    -o %OUT_DIR%\full_pipeline_test.exe
if %ERRORLEVEL% NEQ 0 (
    echo Failed to build full pipeline test
    pause
    exit /b 1
)
%OUT_DIR%\full_pipeline_test.exe
endlocal
