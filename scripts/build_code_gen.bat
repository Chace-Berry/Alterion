@echo off
echo Building Alterion Code Generation System...
echo.

REM Create build directory
if not exist "build" mkdir build

REM Build the code generation test
echo Compiling code generation test...
g++ -std=c++17 -I"include" -O2 -Wall -Wextra ^
    tests/code_gen_test.cpp ^
    src/ast_implementation.cpp ^
    -o build/code_gen_test.exe

if %ERRORLEVEL% NEQ 0 (
    echo Failed to build code generation test
    pause
    exit /b 1
)

REM Build the Alterion compiler
echo Compiling Alterion compiler...
g++ -std=c++17 -I"include" -O2 -Wall -Wextra ^
    src/alterion_compiler.cpp ^
    src/ast_implementation.cpp ^
    -o build/alterion.exe

if %ERRORLEVEL% NEQ 0 (
    echo Failed to build Alterion compiler
    pause
    exit /b 1
)

echo.
echo ✅ Build successful!
echo.

REM Run code generation tests
echo Running code generation tests...
echo.
build\code_gen_test.exe

echo.
echo Testing Alterion compiler...
echo.

REM Test the compiler with our demo app
echo Compiling demo app to TypeScript...
build\alterion.exe -t ts -v examples/demo_app.alt

if exist "dist/demo_app.ts" (
    echo ✅ Demo app compiled successfully!
    echo Generated file: dist/demo_app.ts
    echo.
    echo First 20 lines of generated TypeScript:
    powershell "Get-Content dist/demo_app.ts | Select-Object -First 20"
) else (
    echo ❌ Demo app compilation failed
)

echo.
echo Testing JavaScript generation...
build\alterion.exe -t js -o dist/demo_app.js examples/demo_app.alt

if exist "dist/demo_app.js" (
    echo ✅ JavaScript generation successful!
    echo Generated file: dist/demo_app.js
) else (
    echo ❌ JavaScript generation failed
)

echo.
echo Testing React Native generation...
build\alterion.exe -t rn -o dist/demo_app.tsx examples/demo_app.alt

if exist "dist/demo_app.tsx" (
    echo ✅ React Native generation successful!
    echo Generated file: dist/demo_app.tsx
) else (
    echo ❌ React Native generation failed
)

echo.
echo 🎉 Phase 2D: Code Generation System Complete!
echo.
echo Available executables:
echo - build\alterion.exe          (Alterion compiler)
echo - build\code_gen_test.exe     (Code generation tests)
echo.
echo Generated files:
dir /b dist\*.* 2>nul

pause
