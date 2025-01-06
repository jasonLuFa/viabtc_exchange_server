# matchengine

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
    participant Timer as Timer System
    
    %% Initialization Phase
    M->>DB: get_last_slice() from `slice_history` table
    DB-->>M: last_slice_time, last_oper_id, last_order_id
    
    M->>DB: load_slice_from_db(last_slice_time)
    activate DB
    DB->>DB: `slice_order_{timestamp}` table 搜尋 `訂單快照` 資料
    DB->>DB: `slice_balance_{timestamp}` table 搜尋 `使用者資產` 快照資料
    DB-->>M: Slice Data
    deactivate DB
    
    M->>Mem: Initialize Memory Structures
    
    M->>DB: load_operlog_from_db() 避免 operlog table 有紀錄，但未反映在 balance 與 order table
    activate DB
    DB-->>M: Operations since last slice
    M->>Mem: Apply Operations
    
    %% Runtime Operations
    loop Every Trade Operation
        M->>Mem: Update Balance
        M->>Mem: Record Operation
    end
    
    %% Periodic Snapshot (slice_interval)
    loop Every 3600 seconds
        Timer->>M: Trigger dump_to_db()
        M->>DB: Create slice_order_timestamp
        M->>DB: Create slice_balance_timestamp
        M->>Mem: Get Current State
        M->>DB: Dump Orders
        M->>DB: Dump Balances
        M->>DB: Update slice_history
    end
    
    %% Cleanup Old Slices
    loop Every cleanup interval
        Timer->>M: Check Old Slices
        M->>DB: Find Expired Slices
        M->>DB: Delete Old slice_order
        M->>DB: Delete Old slice_balance
        M->>DB: Update slice_history
    end
```