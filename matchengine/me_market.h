/*
 * Description: 
 *     History: yang@haipo.me, 2017/03/16, create
 */

# ifndef _ME_MARKET_H_
# define _ME_MARKET_H_

# include "me_config.h"

extern uint64_t order_id_start;
extern uint64_t deals_id_start;

typedef struct order_t {
    // 基本信息
    uint64_t        id;
    uint32_t        type; // 訂單類型（1: MARKET_ORDER_TYPE_LIMIT、2: MARKET_ORDER_TYPE_MARKET）
    uint32_t        side; // 買賣方向 (1: MARKET_ORDER_SIDE_ASK, 2: MARKET_ORDER_SIDE_BID)
    double          create_time; // 訂單創建時間
    double          update_time; // 訂單最後更新時間
    // 用戶和市場信息
    uint32_t        user_id;
    char            *market; // 交易市場（如：BTC/USDT）
    char            *source; // 訂單來源
    // 價格和數量信息
    mpd_t           *price;  // 訂單價格
    mpd_t           *amount; // 訂單量
    // 手續費相關
    mpd_t           *taker_fee; // taker 手續費率（主動成交方）
    mpd_t           *maker_fee; // maker 手續費率（被動成交方）
    // 訂單狀態追踪
    mpd_t           *left;   // 剩餘未成交數量
    mpd_t           *freeze; // 凍結的資金數量
    // 成交統計
    mpd_t           *deal_stock; // 已成交的基礎貨幣數量 (如：BTC/USDT 基礎貨幣是 BTC)
    mpd_t           *deal_money; // 已成交的計價貨幣數量（如：BTC/USDT 計價貨幣是 USDT)
    mpd_t           *deal_fee;   // 已產生的手續費
} order_t;

typedef struct market_t {
    char            *name;  // Market name (e.g., "BTCUSDT")
    char            *stock; // Base asset (e.g., "BTC")
    char            *money; // Quote asset (e.g., "USDT")

    int             stock_prec; // Precision for base asset amount (e.g., 8 for BTC)
    int             money_prec; // Precision for quote asset amount (e.g., 2 for USDT)
    int             fee_prec;   // Precision for fees calculations (e.g., 8 for 0.1%)
    mpd_t           *min_amount; // Minimum trade amount (e.g., 0.0001 BTC)

    dict_t          *orders; // All orders in the market indexed by order ID
    dict_t          *users;  // All users in the market indexed by user ID

    skiplist_t      *asks;   // Sell limit orders sorted by price (ascending)
    skiplist_t      *bids;   // Buy limit orders sorted by price (descending)
} market_t;

market_t *market_create(struct market *conf);
int market_get_status(market_t *m, size_t *ask_count, mpd_t *ask_amount, size_t *bid_count, mpd_t *bid_amount);

int market_put_limit_order(bool real, json_t **result, market_t *m, uint32_t user_id, uint32_t side, mpd_t *amount, mpd_t *price, mpd_t *taker_fee, mpd_t *maker_fee, const char *source);
int market_put_market_order(bool real, json_t **result, market_t *m, uint32_t user_id, uint32_t side, mpd_t *amount, mpd_t *taker_fee, const char *source);
int market_cancel_order(bool real, json_t **result, market_t *m, order_t *order);

int market_put_order(market_t *m, order_t *order);

json_t *get_order_info(order_t *order);
order_t *market_get_order(market_t *m, uint64_t id);
skiplist_t *market_get_order_list(market_t *m, uint32_t user_id);

sds market_status(sds reply);

# endif

