#!/bin/bash

# Clean previous run
rm -f client*_file*

# Compile
mpic++ -o tema2 ../tema2.cpp -pthread -Wall -lmpi_cxx

# Run with 4 processes (1 tracker + 3 clients)
mpirun -np 4 ./tema2

# Check results
echo "=== Checking output files ==="
for f in client*_file*; do
    if [ -f "$f" ]; then
        echo "Found file: $f"
        echo "Number of lines: $(wc -l < "$f")"
        echo "First few lines:"
        head -n 3 "$f"
        echo "---"
    else
        echo "Missing expected file: $f"
    fi
done