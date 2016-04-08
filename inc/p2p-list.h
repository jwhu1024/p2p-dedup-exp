#ifndef _P2P_LIST_H_
#define _P2P_LIST_H_

#include <time.h>
#include <stdbool.h>

struct ip_list_struct {
	char ipaddr[17];
	time_t ts;
	bool isSuper;
	struct ip_list_struct *next;
};

struct ip_list_struct* list_add (struct ip_list_struct* ip_s, bool add_to_end);
struct ip_list_struct* list_search_by_ip (char *ip, struct ip_list_struct **prev);
int list_delete_by_ip (char *ip);
int self_test (void);
int list_count (void);
void list_free (void);
void list_display (void);
void list_check_timestamp (time_t cur_time, int threshold);
char *list_get_ip_by_index(int);

#endif /* _P2P_LIST_H_ */