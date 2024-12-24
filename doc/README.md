# doc

- [doc](#doc)
  - [啟動環境](#啟動環境)
  - [檢查環境](#檢查環境)
    - [Kafka](#kafka)
  - [檢查 ws](#檢查-ws)
  - [Redis](#redis)
    - [Architecture](#architecture)
    - [Validation Commands](#validation-commands)
      - [1. Check Redis Master Status](#1-check-redis-master-status)
      - [2. Check Redis Slave Status](#2-check-redis-slave-status)
      - [3. Check Sentinel Status](#3-check-sentinel-status)

## 啟動環境

1. `./init_container.sh`
2. 連到 container 中
    - 安裝 extension [Remote - Containers](https://marketplace.visualstudio.com/items?itemName=ms-vscode-remote.remote-containers)，連到該 Container 直接開發 ( 後續步驟都以 Remote - Containers 開發為範例 )
    - 可以使用 `docker exec -it viabtc-ubuntu-container /bin/bash` 可以進入 Container 使用 terminal
3. 編譯 c 程式：直接在 /workspace 執行 `./compile.sh`
4. 

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

## 檢查 ws

- `docker exec viabtc-ubuntu-container python3 /workspace/test/accessws/ws_test.py` 


## Redis 

The system uses Redis in a high-availability configuration with master-slave replication and sentinel monitoring. This setup ensures data persistence and automatic failover in case of master node failure.

### Architecture
- 1 Redis master (port 6379)
- 1 Redis slave (port 6380)
- 3 Redis sentinels (ports 26381, 26382, 26383)

### Validation Commands

#### 1. Check Redis Master Status
```bash
# Check master replication status

# Expected output should show:
# role:master
# connected_slaves:1
```

#### 2. Check Redis Slave Status
```bash
# Check slave replication status
docker exec redis-slave redis-cli info replication

# Expected output should show:
# role:slave
# master_host:redis-master
# master_port:6379
# master_link_status:up
```

#### 3. Check Sentinel Status
```bash
# Check sentinel status (can be run on any sentinel)
docker exec redis-sentinel-1 redis-cli -p 26381 sentinel master mymaster
docker exec redis-sentinel-1 redis-cli -p 26381 sentinel master mymaster

# Expected output should show:
# name: mymaster
# ip: <redis-master-ip>
# port: 6379
# quorum: 2
```

- `docker exec redis-master redis-cli info replication`: Check Redis Master Status
- `docker exec redis-slave redis-cli info replication`: Check Redis Slave Status
- `docker exec redis-sentinel-1 redis-cli -p 26381 info sentinel`: Check Sentinel Status
- `docker exec redis-sentinel-1 redis-cli -p 26381 sentinel master mymaster`: Check Sentinel Master Status
- test data replication by writing to master and reading from slave
  - `docker exec redis-master redis-cli set test_key test_value`: writing to master
  - `docker exec redis-slave redis-cli get test_key`: reading from slave