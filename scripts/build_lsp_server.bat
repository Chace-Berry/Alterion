@echo off
REM Build script for Alterion LSP server phase
setlocal
set INCLUDE_DIR=include
set SRC_DIR=src
set TEST_DIR=tests
set OUT_DIR=build
if not exist %OUT_DIR% mkdir %OUT_DIR%
g++ -std=c++17 -I%INCLUDE_DIR% -I. -O2 -Wall -Wextra \
    lsp_server.cpp \
    %TEST_DIR%\lsp_server_test.cpp \
    -o %OUT_DIR%\lsp_server_test.exe
if %ERRORLEVEL% neq 0 exit /b %ERRORLEVEL%
%OUT_DIR%\lsp_server_test.exe
endlocal
