# doc

- [doc](#doc)
  - [啟動環境](#啟動環境)
  - [開發相關](#開發相關)
  - [檢查環境](#檢查環境)
    - [Kafka](#kafka)
    - [檢查 ws](#檢查-ws)
    - [Redis](#redis)
      - [Architecture](#architecture)
      - [Validation Commands](#validation-commands)
  - [簡單 UI 測試 accesshttp 的 JSON-RPC](#簡單-ui-測試-accesshttp-的-json-rpc)

## 啟動環境

1. `./init_container.sh` ( 後可用 `./restart_container.sh` 省略 build 部分)
2. 連到 container 中
    - 安裝 extension [Remote - Containers](https://marketplace.visualstudio.com/items?itemName=ms-vscode-remote.remote-containers)，連到該 Container 直接開發，因為專案依賴於 linux 環境  ( 後續步驟都以 Remote - Containers linux 開發為前提 )
    - 可以使用 `docker exec -it viabtc-ubuntu-container /bin/bash` 進入 Container 使用 terminal
3. 編譯所有相關 c 程式服務並執行：直接在 /workspace 執行 `./compile.sh`
4. 可以到各服務 `log/xxx.log` 看各服務相關 log

## 開發相關

- 要使用 **debug mode** 必須註解各服務以下兩行，讓程式不會跑在背景

  ```c
  daemon(1, 1);
  process_keepalive();
  ```

- 如程式有修改，必須到該專案底下執行 `make clean && make` 重新編譯程式

## 檢查環境

### Kafka

- `docker exec viabtc-kafka kafka-topics.sh --list --bootstrap-server localhost:9092`：列出所有topic

  ```text
  balances
  deals
  orders
  ```

- `docker exec viabtc-kafka kafka-topics.sh --describe --topic balances --bootstrap-server localhost:9092`：檢查其中一個 topic 的內容

  ```text
  Topic: balances	TopicId: I4PBzJfVT2qpAe25neDY6w	PartitionCount: 1	ReplicationFactor: 1	Configs: segment.bytes=1073741824
    Topic: balances	Partition: 0	Leader: 1001	Replicas: 1001	Isr: 1001
  ```

- `docker exec viabtc-kafka tail -f opt/kafka/logs/server.log`： kafka 服務器日誌

### 檢查 ws

- `docker exec viabtc-ubuntu-container python3 /workspace/test/accessws/ws_test.py`

### Redis

#### Architecture

- The system uses Redis in a high-availability configuration with master-slave replication and sentinel monitoring. This setup ensures data persistence and automatic failover in case of master node failure.
- 1 Redis master (port 6379)
- 1 Redis slave (port 6380)
- 3 Redis sentinels (ports 26381, 26382, 26383)

#### Validation Commands

- `docker exec redis-master redis-cli info replication`: Check Redis Master Status
- `docker exec redis-slave redis-cli info replication`: Check Redis Slave Status
- `docker exec redis-sentinel-1 redis-cli -p 26381 info sentinel`: Check Sentinel Status
- `docker exec redis-sentinel-1 redis-cli -p 26381 sentinel master mymaster`: Check Sentinel Master Status
- test data replication by writing to master and reading from slave
  - `docker exec redis-master redis-cli set test_key test_value`: writing to master
  - `docker exec redis-slave redis-cli get test_key`: reading from slave


## 簡單 UI 測試 accesshttp 的 JSON-RPC

- 到 `test/frontend` 根據 README.md 安裝相關依賴
- 在 `npm run build` 後使用 `./start.sh` 執行 http server
- 使用 `./stop.sh` 停止 http server
