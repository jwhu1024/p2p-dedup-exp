#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "p2p-list.h"
#include "p2p-common.h"

#define L_DEBUG		0

struct ip_list_struct *head = NULL;
struct ip_list_struct *curr = NULL;

struct ip_list_struct* create_list (char *ip)
{
	if (L_DEBUG)
		_dbg("Creating list with headnode as [%s]\n", ip);

	struct ip_list_struct *ptr = (struct ip_list_struct*) malloc (sizeof (struct ip_list_struct));
	if (NULL == ptr) {
		_dbg("Node creation failed \n");
		return NULL;
	}

	memcpy(ptr->ipaddr, ip, strlen(ip) + 1);
	ptr->ts = time(NULL);
	ptr->isSuper = false;
	ptr->next = NULL;

	head = curr = ptr;
	return ptr;
}

struct ip_list_struct* list_add (struct ip_list_struct* ip_s, bool add_to_end)
{
	if (NULL == head) {
		return (create_list(ip_s->ipaddr));
	}

	if (L_DEBUG) {
		if (add_to_end)
			_dbg("Adding node to end of list with value [%s]\n", ip_s->ipaddr);
		else
			_dbg("Adding node to beginning of list with value [%s]\n", ip_s->ipaddr);
	}

	struct ip_list_struct *ptr = (struct ip_list_struct*) malloc (sizeof (struct ip_list_struct));
	if (NULL == ptr) {
		_dbg("Node creation failed \n");
		return NULL;
	}

	memcpy(ptr->ipaddr, ip_s->ipaddr, strlen(ip_s->ipaddr) + 1);
	ptr->ts      = ip_s->ts;
	ptr->isSuper = ip_s->isSuper;
	ptr->next    = NULL;

	if (add_to_end) {
		curr->next = ptr;
		curr = ptr;
	} else {
		ptr->next = head;
		head = ptr;
	}
	return ptr;
}

struct ip_list_struct* list_search_by_ip (char *ip, struct ip_list_struct **prev)
{
	struct ip_list_struct *ptr = head;
	struct ip_list_struct *tmp = NULL;
	bool found = false;

	if (L_DEBUG)
		_dbg("Searching the list for value [%s] \n", ip);

	while (ptr != NULL) {
		if (strcmp(ptr->ipaddr, ip) == 0) {
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

int list_delete_by_ip (char *ip)
{
	struct ip_list_struct *prev = NULL;
	struct ip_list_struct *del = NULL;

	if (L_DEBUG)
		_dbg("Deleting value [%s] from list\n", ip);

	del = list_search_by_ip(ip, &prev);
	if (del == NULL) {
		_dbg("[%s] Not found in list\n", ip);
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
	struct ip_list_struct* tmp;

	while (head != NULL) {
		tmp = head;
		head = head->next;
		free(tmp);
	}

	if (L_DEBUG)
		_dbg("list_free done \n");
}

void list_display (void)
{
	struct ip_list_struct *ptr = head;

	if (L_DEBUG)
		_dbg("%s--------- Printing list Start ---------%s\n", LIGHT_GREEN, RESET);

	while (ptr != NULL)
	{
		if (L_DEBUG) {
			_dbg("%s------------------------%s\n", LIGHT_GREEN, RESET);
			_dbg("%sip -> %s%s\n", LIGHT_GREEN, ptr->ipaddr, RESET);
			_dbg("%sisSuper -> %s%s\n", LIGHT_GREEN, (ptr->isSuper == true) ? "true" : "false", RESET);
			_dbg("%stimestamp -> %d%s\n", LIGHT_GREEN, (int) ptr->ts, RESET);
		}
		ptr = ptr->next;
	}

	if (L_DEBUG)
		_dbg("%s--------- Printing list End -----------%s\n\n", LIGHT_GREEN, RESET);

	return;
}

int list_count ()
{
	int num = 0;
	struct ip_list_struct *ptr = head;

	while (ptr != NULL) {
		ptr = ptr->next;
		num++;
	}

	if (L_DEBUG)
		_dbg("List have %d ip\n\n", num);

	return num;
}

char *list_get_ip_by_index (int idx)
{
	int num = 0;
	struct ip_list_struct *ptr = head;

	while (ptr != NULL) {
		if (idx == num++) {
			return ptr->ipaddr;
		}
		ptr = ptr->next;
	}
	return NULL;
}

void list_check_timestamp (time_t cur_time, int threshold)
{
	struct ip_list_struct *ptr = head;

	while (ptr != NULL) {
		time_t diff = cur_time - ptr->ts;
		if (diff >= threshold) {
			_dbg("%s%s LEAVE%s\n", YELLOW, ptr->ipaddr, RESET);
			list_delete_by_ip(ptr->ipaddr);
		}
		ptr = ptr->next;
	}
	return;
}

/* int self_test (void)
{
	int ret = 0;
	struct ip_list_struct *ptr = NULL;

	list_display();

	list_add("11:22:33:44:55:66", time(NULL), true);
	list_display();
	list_add("33:22:33:44:55:66", time(NULL), true);
	list_add("22:22:33:44:55:66", time(NULL), true);
	list_add("44:22:33:44:55:66", time(NULL), true);
	list_display();

	ptr = list_search_by_ip("44:22:33:44:55:66", NULL);
	if(NULL == ptr) {
		_dbg("Search [ip = 11:22:33:44:55:66] failed, no such element found\n");
	} else {
		_dbg("Search passed [ip = %s]\n", ptr->ipaddr);
	}

	ret = list_delete_by_ip("44:22:33:44:55:66");
	if(ret != 0) {
		_dbg("delete [ip = 44:22:33:44:55:66] failed, no such element found\n");
	} else {
		_dbg("delete [ip = 44:22:33:44:55:66]  passed \n");
	}

	list_display();
	list_free();
	return 0;
} */