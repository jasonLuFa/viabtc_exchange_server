#!/bin/sh
GREEN='\033[0;32m'
NC='\033[0m' # No Color

# Check if server is already running on port 8000
if ! lsof -i:8000 > /dev/null 2>&1; then
    echo "${GREEN}Starting server at http://localhost:8000/index.html, please open with the chrome browser${NC}"
    python3 -m http.server 8000 &
else
    echo "${GREEN}Server already running at http://localhost:8000/index.html${NC}"
fi

# Check if Chrome is already running with disabled security
if ! ps aux | grep "Chrome.*disable-web-security" | grep -v grep > /dev/null; then
    open -na Google\ Chrome --args --user-data-dir=/tmp/chrome-dev --disable-web-security --allow-file-access-from-files
fi
