# alertcenter

- 以 TCP/UDP 伺服器模式運行於 4444 埠
  - 根據 config.json 的設定
- 使用 Redis 進行警報數據存儲 
  - **ac_server.c** 中 *on_recev_package* 處理
- 能夠透過電子郵件發送警報
  - AWS SES Email
  - 使用 **send_alert.py**
- 處理來自其他服務的全系統警報

```mermaid
graph LR
    A[Other Services] --> B[UDP Message]
    B --> C[alertcenter, port 4444]
    C --> D[Redis Queue]
    D --> E[send_alert.py]
    E --> F[AWS SES Email]
```