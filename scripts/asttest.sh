#!/bin/bash
# Run the AST test and output results to results/ast-results.json
OUTDIR="results"
mkdir -p "$OUTDIR"
core/Debug/asttest.exe > "$OUTDIR/ast-results.json"
