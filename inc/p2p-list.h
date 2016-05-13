#ifndef _P2P_LIST_H_
#define _P2P_LIST_H_

#include <time.h>
#include <stdbool.h>
#include "p2p-common.h"

struct sh_tbl {
	char uuid[SP_PEER_UUID_LENGTH+1];
	unsigned char short_hash[SHORT_HASH_LENGTH+1];
	struct sh_tbl *next;
};

struct sh_tbl* list_add (struct sh_tbl* tbl_s, bool add_to_end);
struct sh_tbl* list_search_by_shorthash (char *uuid, struct sh_tbl **prev);
int list_delete_by_shorthash (char *sh);
int list_self_test (void);
int list_count (void);
void list_free (void);
void list_display (void);
void list_check_timestamp (time_t cur_time, int threshold);
struct sh_tbl *list_get_by_index (int idx);
int list_update_by_shorthash (struct sh_tbl *list);
void list_delete_by_uuid (char *uuid);

#endif /* _P2P_LIST_H_ */