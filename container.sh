#!/bin/sh

# 定義映像名稱和容器名稱
IMAGE_NAME="viabtc-ubuntu-image"
CONTAINER_NAME="viabtc-ubuntu-container"

# 構建 Docker 映像，指定上下文為當前目錄
docker build -f .devcontainer/Dockerfile -t $IMAGE_NAME .


# 檢查是否已存在同名容器，若存在則先刪除
if [ "$(docker ps -a -q -f name=$CONTAINER_NAME)" ]; then
    docker rm -f $CONTAINER_NAME
fi

# 運行 Docker 容器，將當前目錄掛載到 /workspace
docker run -it -d --name $CONTAINER_NAME -v "$(pwd)":/workspace $IMAGE_NAME
