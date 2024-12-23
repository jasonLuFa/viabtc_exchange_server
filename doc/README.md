# doc

- [doc](#doc)
  - [啟動環境](#啟動環境)
  - [檢查環境](#檢查環境)
    - [Kafka](#kafka)

## 啟動環境
1. `./init_container.sh`
2. 連到 container 中
    - 安裝 extension [Remote - Containers](https://marketplace.visualstudio.com/items?itemName=ms-vscode-remote.remote-containers)，連到該 Container 直接開發
    - 可以使用 `docker exec -it viabtc-ubuntu-container /bin/bash` 可以進入 Container 使用 terminal

## 檢查環境

### Kafka

- `docker exec viabtc-kafka kafka-topics.sh --list --bootstrap-server localhost:9092`：列出所有topic

  ```
  balances
  deals
  orders
  ```

- `docker exec viabtc-kafka kafka-topics.sh --describe --topic balances --bootstrap-server localhost:9092`：檢查其中一個 topic 的內容

  ```
  Topic: balances	TopicId: I4PBzJfVT2qpAe25neDY6w	PartitionCount: 1	ReplicationFactor: 1	Configs: segment.bytes=1073741824
    Topic: balances	Partition: 0	Leader: 1001	Replicas: 1001	Isr: 1001
  ```

- `docker exec viabtc-kafka tail -f opt/kafka/logs/server.log`： kafka 服務器日誌