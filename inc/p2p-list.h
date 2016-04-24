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
char *list_get_ip_by_index(int);

#endif /* _P2P_LIST_H_ */