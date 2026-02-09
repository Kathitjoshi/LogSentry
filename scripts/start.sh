#!/bin/bash

echo "Starting Distributed Log Analyzer..."

# Check if Docker is available
if command -v docker-compose &> /dev/null; then
    echo "Using Docker Compose..."
    docker-compose up -d
    echo "Services started! Access dashboard at http://localhost:5000"
else
    echo "Docker not found. Please install Docker or run services manually."
    echo "See README.md for manual setup instructions."
fi
