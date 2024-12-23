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