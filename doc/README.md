# doc

- [doc](#doc)
  - [啟動環境](#啟動環境)
  - [開發相關](#開發相關)
  - [檢查環境](#檢查環境)
    - [Kafka](#kafka)
    - [檢查 ws](#檢查-ws)
    - [Redis](#redis)
  - [簡單 UI 測試 accesshttp 的 JSON-RPC](#簡單-ui-測試-accesshttp-的-json-rpc)
    - [啟動 UI](#啟動-ui)
    - [測試步驟](#測試步驟)
  - [不同服務介紹](#不同服務介紹)

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
- marketprice 必須在 matchengine 後面啟動，來取得 `market.list` 的 request

## 檢查環境

### Kafka

- `localhost:8090` 開啟 Kafka UI
- 驗證
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

- 系統使用 Redis 的高可用性配置，採用主從複製和哨兵監控。這種設置確保數據持久性並在主節點故障時自動進行故障切換。
  - 1 個 Redis 主節點（端口 6379）
  - 1 個 Redis 從節點 (utils 裡有相關邏輯，但沒有使用)
  - 3 個 Redis 哨兵（端口 26381、26382、26383）

- 驗證指令
  - `docker exec redis-master redis-cli info replication`: Check Redis Master Status
  - `docker exec redis-slave redis-cli info replication`: Check Redis Slave Status
  - `docker exec redis-sentinel-1 redis-cli -p 26381 info sentinel`: Check Sentinel Status
  - `docker exec redis-sentinel-1 redis-cli -p 26381 sentinel master mymaster`: Check Sentinel Master Status
  - test data replication by writing to master and reading from slave
    - `docker exec redis-master redis-cli set test_key test_value`: writing to master
    - `docker exec redis-slave redis-cli get test_key`: reading from slave


## 簡單 UI 測試 accesshttp 的 JSON-RPC

### 啟動 UI

- 到 `test/frontend` 根據 README.md 安裝相關依賴
- 在 `npm run build` 後使用 `./start.sh` 執行 http server
- 使用 `./stop.sh` 停止 http server

### 測試步驟

1. 先到 **Balance tag** 使用 *Balance Update* 初始化兩個使用者 balance
2. 再到 **Trading tag** 使用下訂單 ( 這邊下單的幣種必須是在 matchengine 的 config.json 中的 markets 有配置的才行 )

## 不同服務介紹

- [alertcenter](alertcenter.md)
- [matchengine](matchengine.md)
- [marketprice](marketprice.md)
- [readhistory](readhistory.md)
- [accesshttp](accesshttp.md)
- [accessws](accessws.md)
