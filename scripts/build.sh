#!/bin/bash
set -e

echo "Building KV Store..."
cd kvstore
mkdir -p build
cd build
cmake ..
make
cd ../..

echo "Installing Python dependencies..."
pip install -r requirements.txt

echo "Build complete!"
