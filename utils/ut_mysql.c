# include "ut_log.h"
# include "ut_mysql.h"
# include <stdbool.h>

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
