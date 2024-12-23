#!/bin/sh

cd .devcontainer

echo "Stopping existing containers..."
docker-compose down -v

echo "Building and starting containers..."
docker-compose up -d

echo "\nTo enter the development container, you can:"
echo "1. Use VS Code with Remote-Containers extension (recommended)"
echo "2. Run: docker exec -it viabtc-ubuntu-container /bin/bash"
