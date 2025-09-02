#!/bin/bash

# Alterion Language Lexer and Parser Build Script

echo "Building Alterion Language Lexer and Parser..."

# Create build directory if it doesn't exist
mkdir -p build

# Compile the complete test with all dependencies
g++ -std=c++17 -I"include" -O2 -Wall -Wextra \
    tests/complete_test.cpp \
    -o build/alterion_test

if [ $? -eq 0 ]; then
    echo "Build successful!"
    echo "Running tests..."
    echo "===================="
    ./build/alterion_test
else
    echo "Build failed!"
    exit 1
fi
