#!/bin/sh/
cd "$(dirname "$0")/.."
g++ -std=c++17 -Icore/include tests/unit/lexertest.cpp core/lexer/lexer.cpp -o lexertest
./lexertest
cat lexer_test_output.csv
