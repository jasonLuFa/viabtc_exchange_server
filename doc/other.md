# Other

- [Other](#other)
  - [JSON RPC 介紹](#json-rpc-介紹)
  - [非同步事件處理機制介紹（nw\_state）](#非同步事件處理機制介紹nw_state)

## JSON RPC 介紹

- 基於 TCP 的可靠傳輸
- 使用流程

  ```c
  // 1. 定義配置
  rpc_clt_cfg cfg = {
      .name = "matchengine",
      .addr_count = 1,
      .addr_arr = &addr,
      .sock_type = SOCK_STREAM,
  };

  // 2. 定義回調
  rpc_clt_type type = {
      .on_connect = your_connect_handler,
      .on_recv_pkg = your_recv_handler
  };

  // 3. 創建客戶端
  rpc_clt *clt = rpc_clt_create(&cfg, &type);

  // 4. 發送請求
  rpc_pkg pkg = {
      .pkg_type = RPC_PKG_TYPE_REQUEST,
      .command = CMD_BALANCE_QUERY,
      .sequence = sequence_id,
      .body = json_dumps(params, 0)
  };
  rpc_clt_send(clt, &pkg);

  // 5. 處理連接（在回調中）
  static void your_connect_handler(nw_ses *ses, bool result)
  {
      // 處理連接...
  }

  // 6. 處理響應（在回調中）
  static void your_recv_handler(nw_ses *ses, rpc_pkg *pkg)
  {
      // 處理響應...
  }
  ```

## 非同步事件處理機制介紹（nw_state）

- 使用哈希表存儲狀態
- 每個狀態有獨立的定時器
- 通過 ID 快速查找狀態
- 自動處理超時和清理
- 使用流程

  ```c
  // 1. 定義狀態類型
  nw_state_type type = {
      .on_timeout = your_timeout_handler,    // 必須實現
      .on_release = your_release_handler     // 可選
  };

  // 2. 創建狀態管理器
  struct state_data {
      uint32_t user_id;
      char asset[16];
  };
  nw_state *state = nw_state_create(&type, sizeof(struct state_data));

  // 3. 添加狀態
  nw_state_entry *entry = nw_state_add(state, 5.0, 0);  // 5秒超時
  struct state_data *data = entry->data;
  data->user_id = 1001;
  strcpy(data->asset, "BTC");

  // 4. 查找狀態
  nw_state_entry *found = nw_state_get(state, entry->id);

  // 5. 刪除狀態
  nw_state_del(state, entry->id);

  // 6. 釋放資源
  nw_state_release(state);
  ```