# include "ut_log.h"
# include "ut_mysql.h"
# include <stdbool.h>
# include <unistd.h>

MYSQL *mysql_connect(mysql_cfg *cfg)
{
    MYSQL *conn = mysql_init(NULL);
    if (conn == NULL){
        return NULL;
    }

    // 設置字符集
    if (mysql_options(conn, MYSQL_SET_CHARSET_NAME, cfg->charset) != 0) {
        mysql_close(conn);
        return NULL;
    }
    // 設置連接超時
    unsigned int timeout = 10;
    if (mysql_options(conn, MYSQL_OPT_CONNECT_TIMEOUT, &timeout) != 0) {
        mysql_close(conn);
        return NULL;
    }
    // 設置讀寫超時
    unsigned int read_timeout = 10;
    unsigned int write_timeout = 10;
    mysql_options(conn, MYSQL_OPT_READ_TIMEOUT, &read_timeout);
    mysql_options(conn, MYSQL_OPT_WRITE_TIMEOUT, &write_timeout);
    // 確認連線成功
    // CLIENT_MULTI_STATEMENTS 允許多語句查詢，CLIENT_REMEMBER_OPTIONS 保持連線選項一致性
    bool connectionFailed = (mysql_real_connect(conn, cfg->host, cfg->user, cfg->pass, cfg->name, cfg->port, NULL, 
        CLIENT_MULTI_STATEMENTS | CLIENT_REMEMBER_OPTIONS) == NULL);
        
    if (connectionFailed) {
        mysql_close(conn);
        return NULL;
    }

    // 移除棄用的重連選項，直接在建立連接時處理

    return conn;
}

/**
 * 帶重試機制的執行 SQL 函式
 * @param conn         指向 MYSQL* 的指標
 * @param cfg          MySQL 設定結構
 * @param sql          要執行的 SQL 字串 (sds 型態)
 * @param max_retries  最大重試次數
 * @return             0 表示成功，非 0 表示失敗
 */
int exec_sql_with_retry(MYSQL **conn, mysql_cfg *cfg, sds sql, int max_retries)
{
    for (int attempt = 0; attempt <= max_retries; attempt++) {
        int query_ret = mysql_real_query(*conn, sql, sdslen(sql));
        if (query_ret == 0) {
            return 0;
        }

        // 查詢失敗，取得錯誤代碼與訊息
        unsigned int err_code = mysql_errno(*conn);
        const char *err_msg = mysql_error(*conn);
        log_error("執行 SQL: %s 失敗, err_code=%d, err_msg=%s", sql, err_code, err_msg);

        // 非斷線錯誤，不重試
        if (err_code != CR_SERVER_LOST && err_code != CR_SERVER_GONE_ERROR) {
            return err_code;
        }

        // 檢查是否已達最大重試次數
        if (attempt == max_retries) {
            log_fatal("已達到最大重試次數 (%d)。放棄執行。", max_retries);
            break;
        }

        // 嘗試重連
        log_debug("嘗試重連... (第 %d 次嘗試 / 最大 %d 次)", attempt + 1, max_retries);
        mysql_close(*conn);
        *conn = mysql_connect(cfg);

        if (*conn == NULL) {
            log_debug("重連嘗試 %d 失敗。", attempt + 1);
            sleep(1); // 暫停 1 秒
            continue; // 繼續下一次重試
        }

        log_debug("重連嘗試 %d 成功。", attempt + 1);
    }

    return mysql_errno(*conn);
}

bool is_table_exists(MYSQL *conn, const char *table)
{
    sds sql = sdsempty();
    sql = sdscatprintf(sql, "SHOW TABLES LIKE '%s'", table);
    log_trace("exec sql: %s", sql);
    int ret = mysql_real_query(conn, sql, sdslen(sql));
    if (ret < 0) {
        log_error("exec sql: %s fail: %d %s", sql, mysql_errno(conn), mysql_error(conn));
        sdsfree(sql);
        return false;
    }
    sdsfree(sql);

    MYSQL_RES *result = mysql_store_result(conn);
    size_t num_rows = mysql_num_rows(result);
    mysql_free_result(result);
    if (num_rows == 1)
        return true;

    return false;
}
