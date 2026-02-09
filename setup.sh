#!/bin/bash
# Setup script for Distributed Log Analyzer

set -e

echo "=========================================="
echo "Distributed Log Analyzer - Setup"
echo "=========================================="
echo ""

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Check for required tools
echo "Checking for required tools..."

command -v cmake >/dev/null 2>&1 || {
    echo -e "${RED}✗ cmake not found${NC}"
    echo "  Install with: sudo apt-get install cmake"
    exit 1
}
echo -e "${GREEN}✓ cmake found${NC}"

command -v g++ >/dev/null 2>&1 || {
    echo -e "${RED}✗ g++ not found${NC}"
    echo "  Install with: sudo apt-get install g++"
    exit 1
}
echo -e "${GREEN}✓ g++ found${NC}"

command -v python3 >/dev/null 2>&1 || {
    echo -e "${RED}✗ python3 not found${NC}"
    exit 1
}
echo -e "${GREEN}✓ python3 found${NC}"

command -v docker >/dev/null 2>&1 || {
    echo -e "${YELLOW}⚠ docker not found (optional for Docker deployment)${NC}"
} && echo -e "${GREEN}✓ docker found${NC}"

echo ""
echo "Installing Python dependencies..."
python3 -m pip install -r requirements.txt

echo ""
echo "Building C++ KV Store..."
cd kvstore
mkdir -p build
cd build
cmake ..
make

echo ""
echo "Running tests..."
ctest --output-on-failure || echo -e "${YELLOW}Some tests failed, but continuing...${NC}"

echo ""
echo "Building Python bindings..."
cd ../..
cd kvstore
python3 -m pip install . || echo -e "${YELLOW}Python bindings build failed (may need pybind11)${NC}"
cd ..

echo ""
echo "Setting up directories..."
mkdir -p data
mkdir -p logs

echo ""
echo -e "${GREEN}=========================================="
echo "Setup Complete!"
echo "==========================================${NC}"
echo ""
echo "Next steps:"
echo "1. Start RabbitMQ:"
echo "   docker run -d -p 5672:5672 -p 15672:15672 rabbitmq:3-management"
echo ""
echo "2. Start the log consumer:"
echo "   python3 log_processor/consumer/log_consumer.py"
echo ""
echo "3. Start the dashboard:"
echo "   python3 dashboard/app.py"
echo ""
echo "4. Start the log producer (for testing):"
echo "   python3 log_processor/producer/sample_app.py"
echo ""
echo "Or use Docker Compose:"
echo "   docker-compose up"
echo ""
echo "Dashboard will be available at: http://localhost:5000"
echo "RabbitMQ Management: http://localhost:15672 (guest/guest)"
