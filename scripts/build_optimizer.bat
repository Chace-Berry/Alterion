@echo off
REM Build script for Alterion optimizer phase
setlocal
set INCLUDE_DIR=include
set SRC_DIR=src
set TEST_DIR=tests
set OUT_DIR=build
if not exist %OUT_DIR% mkdir %OUT_DIR%
g++ -std=c++17 -I%INCLUDE_DIR% -I. -O2 -Wall -Wextra \
    optimizer.cpp \
    %TEST_DIR%\optimizer_test.cpp \
    -o %OUT_DIR%\optimizer_test.exe
if %ERRORLEVEL% neq 0 exit /b %ERRORLEVEL%
%OUT_DIR%\optimizer_test.exe
endlocal
