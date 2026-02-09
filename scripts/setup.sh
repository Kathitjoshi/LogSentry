#!/bin/bash
set -e

echo "Setting up Distributed Log Analyzer..."

# Check dependencies
command -v g++ >/dev/null 2>&1 || { echo "g++ required but not installed. Aborting." >&2; exit 1; }
command -v cmake >/dev/null 2>&1 || { echo "cmake required but not installed. Aborting." >&2; exit 1; }
command -v python3 >/dev/null 2>&1 || { echo "python3 required but not installed. Aborting." >&2; exit 1; }

# Build C++ components
./scripts/build.sh

# Create data directory
mkdir -p data

echo "Setup complete! Run './scripts/start.sh' to start the system."
