#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "p2p-list.h"
#include "p2p-common.h"

#define L_DEBUG		1

struct sh_tbl *head = NULL;
struct sh_tbl *curr = NULL;

/*
	unsigned char short_hash[SHORT_HASH_LENGTH];
	char uuid[SP_PEER_UUID_LENGTH];
*/
struct sh_tbl* create_list (char *uuid, unsigned char *sh)
{
	if (L_DEBUG)
		DBG ("Creating list with headnode as [%s] - uuid [%s]\n", uuid, sh);

	struct sh_tbl *ptr = (struct sh_tbl*) malloc (sizeof (struct sh_tbl));
	if (NULL == ptr) {
		DBG ("Node creation failed \n");
		return NULL;
	}

	memcpy(ptr->uuid, uuid, SP_PEER_UUID_LENGTH);
	memcpy(ptr->short_hash, sh, SHORT_HASH_LENGTH);
	ptr->next = NULL;

	head = curr = ptr;
	return ptr;
}

struct sh_tbl* list_add (struct sh_tbl* sh_tbl, bool add_to_end)
{
	struct sh_tbl *prev = NULL;
	struct sh_tbl *target = NULL;

	target = list_search_by_shorthash ((char *) sh_tbl->short_hash, &prev);
	if (target != NULL) {
		target = NULL;
		return NULL;
	}

	if (NULL == head) {
		return (create_list(sh_tbl->uuid, sh_tbl->short_hash));
	}

	if (L_DEBUG) {
		if (add_to_end)
			DBG ("Adding node to end of list with value [%s]\n", sh_tbl->uuid);
		else
			DBG ("Adding node to beginning of list with value [%s]\n", sh_tbl->uuid);
	}

	struct sh_tbl *ptr = (struct sh_tbl*) malloc (sizeof (struct sh_tbl));
	if (NULL == ptr) {
		DBG ("Node creation failed \n");
		return NULL;
	}

	memcpy(ptr->uuid, sh_tbl->uuid, SP_PEER_UUID_LENGTH);
	memcpy(ptr->short_hash, sh_tbl->short_hash, SHORT_HASH_LENGTH);
	ptr->next = NULL;

	if (add_to_end) {
		curr->next = ptr;
		curr = ptr;
	} else {
		ptr->next = head;
		head = ptr;
	}
	return ptr;
}

struct sh_tbl* list_search_by_shorthash (char *sh, struct sh_tbl **prev)
{
	struct sh_tbl *ptr = head;
	struct sh_tbl *tmp = NULL;
	bool found = false;

	if (L_DEBUG)
		DBG ("Searching the list for value [%s] \n", sh);

	while (ptr != NULL) {
		if (strcmp((char *) ptr->short_hash, sh) == 0) {
			found = true;
			break;
		} else {
			tmp = ptr;
			ptr = ptr->next;
		}
	}

	if (true == found) {
		if (prev)
			*prev = tmp;
		return ptr;
	} else {
		return NULL;
	}
}

int list_delete_by_shorthash (char *sh)
{
	struct sh_tbl *prev = NULL;
	struct sh_tbl *del = NULL;

	if (L_DEBUG)
		DBG ("Deleting value [%s] from list\n", sh);

	del = list_search_by_shorthash (sh, &prev);
	if (del == NULL) {
		DBG ("[%s] Not found in list\n", sh);
		return -1;
	} else {
		if (prev != NULL)
			prev->next = del->next;

		if (del == curr) {
			curr = prev;
		} else if (del == head) {
			head = del->next;
		}
	}

	free(del);
	del = NULL;

	return 0;
}

void list_free (void)
{
	struct sh_tbl* tmp;

	while (head != NULL) {
		tmp = head;
		head = head->next;
		free(tmp);
	}

	if (L_DEBUG)
		DBG ("list_free done \n");
}

void list_display (void)
{
	struct sh_tbl *ptr = head;

	if (L_DEBUG)
		DBG ("%s--------- Printing list Start ---------%s\n", LIGHT_GREEN, RESET);

	while (ptr != NULL)
	{
		if (L_DEBUG) {
			DBG ("%s------------------------%s\n", LIGHT_GREEN, RESET);
			DBG ("%suuid -> %s%s\n", LIGHT_GREEN, ptr->uuid, RESET);
			DBG ("%sshorthash -> %s%s\n", LIGHT_GREEN, ptr->short_hash, RESET);
		}
		ptr = ptr->next;
	}

	if (L_DEBUG)
		DBG ("%s--------- Printing list End -----------%s\n\n", LIGHT_GREEN, RESET);

	return;
}

int list_count ()
{
	int num = 0;
	struct sh_tbl *ptr = head;

	while (ptr != NULL) {
		ptr = ptr->next;
		num++;
	}

	if (L_DEBUG)
		DBG ("List have %d ip\n\n", num);

	return num;
}

// char *list_get_ip_by_index (int idx)
// {
// 	int num = 0;
// 	struct sh_tbl *ptr = head;

// 	while (ptr != NULL) {
// 		if (idx == num++) {
// 			return ptr->ipaddr;
// 		}
// 		ptr = ptr->next;
// 	}
// 	return NULL;
// }

// void list_check_timestamp (time_t cur_time, int threshold)
// {
// 	struct sh_tbl *ptr = head;

// 	while (ptr != NULL) {
// 		time_t diff = cur_time - ptr->ts;
// 		if (diff >= threshold) {
// 			DBG ("%s%s LEAVE%s\n", YELLOW, ptr->ipaddr, RESET);
// 			list_delete_by_shorthash(ptr->ipaddr);
// 		}
// 		ptr = ptr->next;
// 	}
// 	return;
// }

#if 0 //def __DEBUG__

#include "p2p-encrypt.h"

int list_self_test (void)
{
	// int ret = 0;

	struct sh_tbl *ptr = NULL;
	struct sh_tbl new;

	memset(&new, 0, sizeof(struct sh_tbl));
	memcpy(new.uuid, "B7D1CD4BC21AFEF754108FD06A9D8647", SP_PEER_UUID_LENGTH);
	memcpy(new.short_hash, "010110001101", SHORT_HASH_LENGTH);

	list_add(&new, true);
	list_display();

	memset(&new, 0, sizeof(struct sh_tbl));
	memcpy(new.uuid, "A7D1CD4BC21AFEF754108FD06A9D8647", SP_PEER_UUID_LENGTH);
	memcpy(new.short_hash, "000000000000", SHORT_HASH_LENGTH);

	list_add(&new, true);

	memset(&new, 0, sizeof(struct sh_tbl));
	memcpy(new.uuid, "C7D1CD4BC21AFEF754108FD06A9D8647", SP_PEER_UUID_LENGTH);
	memcpy(new.short_hash, "111111111111", SHORT_HASH_LENGTH);

	list_add(&new, true);
	list_display();

	ptr = list_search_by_shorthash ("000000000000", NULL);
	if (NULL == ptr) {
		DBG ("%sSearch [sh = 000000000000] failed, no such element found%s\n", LIGHT_GREEN, RESET);
	} else {
		DBG ("%sSearch passed [sh = 000000000000, uuid = %s]%s\n", LIGHT_GREEN, ptr->uuid, RESET);
	}

	ptr = list_search_by_shorthash ("111111111111", NULL);
	if (NULL == ptr) {
		DBG ("%sSearch [sh = 111111111111] failed, no such element found%s\n", LIGHT_GREEN, RESET);
	} else {
		DBG ("%sSearch passed [sh = 111111111111, uuid = %s]%s\n", LIGHT_GREEN, ptr->uuid, RESET);
	}

	ptr = list_search_by_shorthash ("010110001101", NULL);
	if (NULL == ptr) {
		DBG ("%sSearch [sh = 010110001101] failed, no such element found%s\n", LIGHT_GREEN, RESET);
	} else {
		DBG ("%sSearch passed [sh = 010110001101, uuid = %s]%s\n", LIGHT_GREEN, ptr->uuid, RESET);
	}

	// ret = list_delete_by_shorthash("010110001101");
	// if(ret != 0) {
	//  	DBG ("%sdelete [sh = 010110001101] failed, no such element found%s\n",LIGHT_GREEN, RESET);
	// } else {
	//  	DBG ("%sdelete [sh = 010110001101]  passed%s\n",LIGHT_GREEN, RESET);
	// }

	// list_display();

	// ret = list_delete_by_shorthash("111111111111");
	// if(ret != 0) {
	//  	DBG ("%sdelete [sh = 111111111111] failed, no such element found%s\n",LIGHT_GREEN, RESET);
	// } else {
	//  	DBG ("%sdelete [sh = 111111111111]  passed%s\n",LIGHT_GREEN, RESET);
	// }

	// list_display();

	// ret = list_delete_by_shorthash("000000000000");
	// if(ret != 0) {
	//  	DBG ("%sdelete [sh = 000000000000] failed, no such element found%s\n",LIGHT_GREEN, RESET);
	// } else {
	//  	DBG ("%sdelete [sh = 000000000000]  passed%s\n",LIGHT_GREEN, RESET);
	// }

	list_display();
	list_free();
	return 0;
}
#endif