#!/bin/bash

# Simple build script for Alterion lexer and parser

echo "Building Alterion lexer and parser..."

# Check if g++ is available
if ! command -v g++ &> /dev/null; then
    echo "g++ not found. Please install a C++ compiler."
    exit 1
fi

# Create build directory
mkdir -p build

# Compile the project
echo "Compiling..."
g++ -I include -std=c++17 -Wall -Wextra \
    lexer/lexer_complete.cpp \
    parser/parser.cpp \
    test_main.cpp \
    -o build/alterion_test

if [ $? -eq 0 ]; then
    echo "Build successful!"
    echo "Executable created: build/alterion_test"
else
    echo "Build failed!"
    exit 1
fi
