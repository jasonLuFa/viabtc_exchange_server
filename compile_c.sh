#!/bin/bash

# Exit on any error
set -e

# Function to build a component
build_component() {
    local component=$1
    echo -e "\n${GREEN}Building ${component}...${NC}"
    cd ${component}
    make clean
    make
    cd ..
}

# Function to start a service
start_service() {
    local service=$1
    local config="config.json"
    echo -e "\n${GREEN}Starting ${service}...${NC}"
    cd ${service}
    # Check if the service binary exists
    if [ ! -f "${service}" ]; then
        echo -e "${RED}Error: ${service} binary not found${NC}"
        cd ..
        return 1
    fi
    # Check if config.json exists
    if [ ! -f "${config}" ]; then
        echo -e "${RED}Error: ${config} not found for ${service}${NC}"
        cd ..
        return 1
    fi
    # Kill existing process if running
    pkill -f "${service}" || true
    # Start the service
    ./${service} ${config} > /dev/null 2>&1 &
    cd ..
    # Wait a bit to ensure service starts
    sleep 1
    # Check if service is running
    if pgrep -f "${service}" > /dev/null; then
        echo -e "${GREEN}${service} started successfully${NC}"
    else
        echo -e "${RED}Failed to start ${service}${NC}"
        return 1
    fi
}

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
NC='\033[0m' # No Color

echo -e "${GREEN}Starting compilation of all components...${NC}"

# Step 1: Build dependencies first
echo -e "\n${GREEN}Building dependencies...${NC}"
cd depends/hiredis
make clean
make
cd ../..

# Step 2: Build utils (required by other components)
build_component "utils"

# Step 3: Build network (required by other components)
build_component "network"

# Step 4: Build all other components
components=(
    "matchengine"
    "marketprice"
    "alertcenter"
    "accesshttp"
    "accessws"
    "readhistory"
)

for component in "${components[@]}"; do
    build_component "$component"
done

echo -e "\n${GREEN}All components have been built successfully!${NC}"

# Step 5: Start all services in the correct order
echo -e "\n${GREEN}Starting all services...${NC}"


# Create necessary log directories
mkdir -p ./matchengine/log
mkdir -p ./marketprice/log
mkdir -p ./alertcenter/log
mkdir -p ./readhistory/log
mkdir -p ./accessws/log
mkdir -p ./accesshttp/log

# Start services in order
for component in "${components[@]}"; do
    start_service "$component"
done

echo -e "\n${GREEN}All services have been started!${NC}"
echo -e "${GREEN}Use 'pgrep -l matchengine' etc. to verify services are running${NC}"