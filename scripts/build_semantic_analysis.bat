@echo off
REM Build script for Alterion semantic analysis phase
setlocal
set INCLUDE_DIR=include
set SRC_DIR=src
set TEST_DIR=tests
set OUT_DIR=build
if not exist %OUT_DIR% mkdir %OUT_DIR%
g++ -std=c++17 -I%INCLUDE_DIR% -I. -O2 -Wall -Wextra \
    semantic_analysis.cpp \
    %TEST_DIR%\semantic_analysis_test.cpp \
    -o %OUT_DIR%\semantic_analysis_test.exe
if %ERRORLEVEL% neq 0 exit /b %ERRORLEVEL%
%OUT_DIR%\semantic_analysis_test.exe
endlocal
