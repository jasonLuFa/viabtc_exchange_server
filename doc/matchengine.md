# matchengine

- [matchengine](#matchengine)
  - [服務自訂 cli 相關](#服務自訂-cli-相關)
  - [balance 和 market 相關儲存流程](#balance-和-market-相關儲存流程)
  - [記憶體相關儲存重要資料結構](#記憶體相關儲存重要資料結構)
    - [Hash Table (dict\_t)](#hash-table-dict_t)
    - [Doubly Linked List (list\_t)](#doubly-linked-list-list_t)
    - [搓和訂單](#搓和訂單)
      - [搓和流程](#搓和流程)
      - [訂單送到 DB](#訂單送到-db)
      - [訂單送到 kafka 流程](#訂單送到-kafka-流程)
  - [排程相關](#排程相關)

## 服務自訂 cli 相關

- 用來即時觀看當前記憶體 balance, market 相關資訊
  - `nc localhost 7317` ( 7317 是 matchengine 的 cli port )
    - `status`
    - `balance list`
    - `balance summary`
    - `balance get <user_id>`
    - `market summary`
    - `makeslice`

## balance 和 market 相關儲存流程

- trade_log 資料庫
  - `operlog_{YYYYMMDD}`：基於 `operlog` 樣板 table，每天建立的 table，用來記錄所有的交易操作
    - 每 0.1 秒批量記錄所有操作紀錄
  - **slice table** 相關，根據 config.json 的 *slice_interval* 的時間間隔建立的 table，並存活 *slice_keeptime* 的時間
    - `slice_order_{timestamp}` table：基於 `slice_order` 樣板 table 建立，用來記錄所有的交易訂單
    - `slice_balance_{timestamp}` table：基於 `slice_balance` 樣板 table 建立，用來記錄所有的使用者資產
  - `slice_history` table 紀錄 `slice_order_{timestamp}` 與 `slice_balance_{timestamp}` 的 timestamp，和 oper, order, deals 的 id

```mermaid
sequenceDiagram
    participant M as Matchengine
    participant Mem as Memory
    participant DB as MySQL
    
    %% Initialization Phase
    M->>DB: get_last_slice()
    activate DB
    DB->>DB: `slice_history` table 
    DB-->>M: last_slice_time, last_oper_id, last_order_id, last_deals_id
    deactivate DB
    
    M->>DB: load_slice_from_db(last_slice_time)
    activate DB
    DB->>DB: `slice_order_{timestamp}` table 搜尋 `訂單` 資料
    DB->>DB: `slice_balance_{timestamp}` table 搜尋 `使用者資產` 快照資料
    DB-->>M: Slice Data
    deactivate DB
    
    M->>Mem: Initialize Slice Data to Memory
    
    loop Replay All Operations not record in snapshot
      M->>DB: load_operlog_from_db() 
      activate DB
      DB-->>M: Operations since last slice
      M-->>DB: 根據 Operations 更新 balance 與 order table
      M->>Mem: 更新當前狀態
      deactivate DB
    end

    %% Runtime Operations
    loop Every Trade Operation
        M->>Mem: Update Balance
        M->>Mem: Record Operation
    end

    loop Every 0.1 seconds
        M->>DB: Batch Update Operation
    end
    
    %% Periodic Snapshot
    loop Every slice_interval Periodic Snapshot
        M->>DB: Create slice_order_{timestamp}
        M->>DB: Create slice_balance_{timestamp}
        Mem-->>M: Get Current State
        M->>DB: Dump slice_order_{timestamp}
        M->>DB: Dump slice_balance_{timestamp}
        M->>DB: Update slice_history
    end
    
    %% Cleanup Old Slices
    loop Every slice_keeptime Cleanup Old Slices
        M->>DB: Find Expired Slices
        M->>DB: Delete Old slice_order
        M->>DB: Delete Old slice_balance
        M->>DB: Update slice_history
    end
```

## 記憶體相關儲存重要資料結構

### Hash Table (dict_t)

- **dict_balance** ( `me_balance.c` )：記錄每個帳戶的餘額
  - Key: {user_id, type, asset}
  - Value: *mpd_t* (decimal balance amount)
- **dict_asset** ( `me_balance.c` )：記錄幣種資訊
  - Key: asset name *string* ( ex: BTC )
  - Value: struct *asset_type* ( asset configuration，來自 config.json 的 assets )
- **dict_market** ( `me_trade.c` )：記錄市場交易對
  - Key: market name *string* ( ex: BTCETH )
  - Value: struct *market_t*  ( market configuration，來自 config.json 的 markets )
- **dict_cache** ( `me_cache.c` )：RPC response cache
  - Key: cache key *string*
  - Value: cached response data
- **dict_sql** ( `me_history.c` )：SQL query cache
  - Key: struct *dict_sql_key* {type, hash}
  - Value: SQL query *string*
- **dict_update** ( `me_update.c` )：記錄近 24 hr 操作行為，避免重複操作（暫時覺得有點多餘，只在初始化和使用者於無變動時，會用到）
  - Key: {user_id, asset, business, business_id}
  - Value: struct *update_val* {create_time}

### Doubly Linked List (list_t)

- **list_deals**: 記錄交易紀錄
- **list_orders**: 記錄訂單紀錄
- **list_balances**: 記錄使用者資產

### 搓和訂單

- **market_t** ( `me_market.h` )：也就是訂單簿(orderbook)，以利於搓和訂單
- market order (市價單)
  - 市價單從不儲存在訂單簿中
  - 它們會立即與現有的限價單進行撮合執行
  - 如果無法完全成交，剩餘的部分將被取消
- limit order (限價單)
  - 當未完全成交時，會儲存在訂單簿中
  - 按照價格和時間優先順序排序
  - 會一直留在訂單簿中，直到以下情況發生：
    - 被後續的訂單完全撮合成交
    - 被用戶取消
    - 到期（如果設定了到期時間）

#### 搓和流程

```mermaid
graph TD
    A[New Order] --> B{Validate Order}
    B -->|Valid| C{Order Type}
    B -->|Invalid| Z[Return Error]
    
    C -->|Market| D[market_put_market_order]
    C -->|Limit| E[market_put_limit_order]
    
    subgraph "Order Matching"
        D --> F{Side?}
        E --> F
        F -->|Ask| G[execute_market/limit_ask_order]
        F -->|Bid| H[execute_market/limit_bid_order]
        
        G & H --> I[Match Against Orderbook]
        I --> J[Update Balances in Memory]
    end
    
    subgraph "Order Updates"
        J --> K[Update Order Status]
        K --> L{Is Limit Order?}
        L -->|Yes| M{Fully Filled?}
        L -->|No| N[Complete Order]
        
        M -->|Yes| N
        M -->|No| O[Add to Orderbook]
    end
    
    subgraph "Persistence & Notification"
        N --> P1[Record History]
        P1 -->|1| Q1[dict_sql]
        P1 -->|2| Q2[Kafka Message]
        
        Q1 --> R1[deal_history]
        Q1 --> R2[user_deal_history]
        Q1 --> R3[balance_history]
        
        Q2 --> S1[deals topic]
        Q2 --> S2[orders topic]
        Q2 --> S3[balances topic]
    end
```

#### 訂單送到 DB

- 存訂單資訊的 sql 會先存到 dict_sql
- 每 0.1 秒執行 dict_sql 儲存的 sql
- 資料表介紹
  - deal_history: 記錄訂單級別的(order_id 索引)成交歷史
  - user_deal_history: 記錄用戶級別(user_deal_history 的 user_id + market 索引)的成交歷史
  - balance_history: 記錄用戶不同資產紀錄 

#### 訂單送到 kafka 流程

- list_t 是用來暫存無法立即送到 kafka 的資料

```mermaid
graph TD
    A[Order Execution] --> B[push_order_message/push_deal_message]
    B --> D{Is list_t empty?}
    D -->|Yes| E[Try produce msg to kafka]
    D -->|No| F[Add to list]
    E -->|Success| G[Free message]
    E -->|Queue Full| F
    
    H[Timer 0.1s] --> J[from memory to produce balance/order/deal message to kafka]
```

- kafka produce deal message ( consumer 在 [marketprice 處理並存到記憶體 market_info.deals](marketprice.md#緩存機制) )

    ```json
    [
        1736492189.0142901,  // [0] 時間戳
        "ETHBCH",            // [1] 市場名稱
        11,                  // [2] ask order id
        10,                  // [3] bid order id
        2,                   // [4] ask user id
        1,                   // [5] bid user id
        "200.00000000",      // [6] 價格
        "1.00000000",        // [7] 數量
        "0.4",               // [8] ask fee (賣方手續費)
        "0.001",             // [9] bid fee (買方手續費)
        1,                   // [10] side (1: sell, 2: buy)
        4,                   // [11] deal id
        "ETH",              // [12] 基準貨幣
        "BCH"               // [13] 計價貨幣
    ]
    ```

## 排程相關

- me_history.c
  - 每 0.1 秒執行 dict_sql 儲存的 sql
- me_main.c
  - 每 0.5 秒將 log 從 buffer 轉存到電腦
- me_message.c
  - 每 0.1 秒將 list_deals, list_orders, list_balances 資料推到 kafka
- me_operlog.c
  - 每 0.1 秒批量更新所有操作紀錄從記憶體( list_t 為 doubly linked list )到 db
- me_persist.c
  - 每 1 秒檢查是否要 snapshot 記憶體 **訂單** (skiplist_t 結構), **使用者資產**(dict_balance) 資訊（更新到 `slice_order_{timestamp}`, `slice_balance_{timestamp}`, `slice_history`）
- me_server.c
  - 每 60 秒清空 dict_cache
- me_update.c
  - 每 60 秒檢查 dict_update 資料是否超過 24 hr，如果超過就刪除
