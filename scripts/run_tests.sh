#!/bin/bash
set -e

echo "Running C++ tests..."
cd kvstore/build
make test
cd ../..

echo "Running Python tests..."
pytest log-processor/tests -v

echo "All tests passed!"
