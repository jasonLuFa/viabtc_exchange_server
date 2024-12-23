#!/bin/sh
RED='\033[0;31m'
NC='\033[0m' # No Color

# Find and kill the Python HTTP server process
if lsof -i:8000 > /dev/null 2>&1; then
    echo "${RED}Stopping server on port 8000...${NC}"
    lsof -ti:8000 | xargs kill
    echo "${RED}Server stopped${NC}"
else
    echo "${RED}No server running on port 8000${NC}"
fi

# Kill any Chrome instances running with disabled security
if ps aux | grep "Chrome.*disable-web-security" | grep -v grep > /dev/null; then
    echo "${RED}Closing insecure Chrome instance...${NC}"
    pkill -f "Chrome.*disable-web-security"
    echo "${RED}Chrome instance closed${NC}"
fi
