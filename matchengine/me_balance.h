/*
 * Description: user balance manage
 *     History: yang@haipo.me, 2017/03/15, create
 */

# ifndef _ME_BALANCE_H_
# define _ME_BALANCE_H_

# include "me_config.h"

# define BALANCE_TYPE_AVAILABLE 1
# define BALANCE_TYPE_FREEZE    2

extern dict_t *dict_balance;

struct balance_key {
    uint64_t    user_id;
    uint32_t    type;
    char        asset[ASSET_NAME_MAX_LEN + 1];
};

int init_balance(void);

bool asset_exist(const char *asset);
int asset_prec(const char *asset);
int asset_prec_show(const char *asset);

mpd_t *balance_get(uint64_t user_id, uint32_t type, const char *asset);
void   balance_del(uint64_t user_id, uint32_t type, const char *asset);
mpd_t *balance_set(uint64_t user_id, uint32_t type, const char *asset, mpd_t *amount);
mpd_t *balance_add(uint64_t user_id, uint32_t type, const char *asset, mpd_t *amount);
mpd_t *balance_sub(uint64_t user_id, uint32_t type, const char *asset, mpd_t *amount);
mpd_t *balance_freeze(uint64_t user_id, const char *asset, mpd_t *amount);
mpd_t *balance_unfreeze(uint64_t user_id, const char *asset, mpd_t *amount);

mpd_t *balance_total(uint64_t user_id, const char *asset);
int balance_status(const char *asset, mpd_t *total, size_t *available_count, mpd_t *available, size_t *freeze_count, mpd_t *freeze);

# endif
