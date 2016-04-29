#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "p2p-event-handler.h"
#include "p2p-list.h"
#include "p2p-cmd-handler.h"

extern sp_info_t sp_info;
extern const char *HEADER_VALUE;
extern const char *P2P_GROUP_NAME;

struct sh_tbl *sh_table = NULL;

int event_enter_handler 	(req_info_t *info);
int event_evasive_handler 	(req_info_t *info);
int event_exit_handler 		(req_info_t *info);
int event_join_handler 		(req_info_t *info);
int event_leave_handler 	(req_info_t *info);
int event_whisper_handler 	(req_info_t *info);
int event_shout_handler 	(req_info_t *info);

zyre_cmd_table_t zyre_op_func_tbl[] = {
	{	"ENTER",		event_enter_handler 	},
	{	"EVASIVE",		event_evasive_handler 	},
	{	"EXIT",			event_exit_handler 		},
	{	"JOIN",			event_join_handler 		},
	{	"LEAVE",		event_leave_handler 	},
	{	"WHISPER",		event_whisper_handler 	},
	{	"SHOUT",		event_shout_handler 	},
	{	"",				NULL 					}
};

void free_mem (req_info_t *info)
{
	if (info->event)	free(info->event);
	if (info->peer)		free(info->peer);
	if (info->name)		free(info->name);
	if (info->group)	free(info->group);
	if (info->message)	free(info->message);
}

static void parse_shout_message (zyre_t *node, char *message);
static void parse_whisper_message (zyre_t *node, char *message);

int event_enter_handler (req_info_t *info)
{
	DBG ("%s%s has joined the group%s\n", YELLOW, info->name, RESET);
	char *header = zyre_peer_header_value (info->node, info->peer, "X-HEADER");

	if (header != NULL) {
#if 0 //def __DEBUG__
		DBG ("%sNAME: %s%s\n", YELLOW, info->name, RESET);
		DBG ("%sPEER: %s%s\n", YELLOW, info->peer, RESET);
		DBG ("%sGROUP: %s%s\n", YELLOW, info->group, RESET);
		DBG ("%sMESSAGE: %s%s\n", YELLOW, info->message, RESET);
		DBG ("%sHEADER: %s%s\n", YELLOW, header, RESET);
#endif
		if (strcmp (header, HEADER_VALUE) == 0) {
			sprintf (sp_info.sp_peer, "%s", info->peer);
			DBG ("%s%s is a superpeer: %s%s\n", YELLOW, info->name, sp_info.sp_peer, RESET);
		}
	} else {
		DBG ("%sHEADER: NOT FOUND%s\n", YELLOW, RESET);
	}

	free_mem(info);
	return 1;
}

int event_evasive_handler (req_info_t *info)
{
	DBG ("%s is being evasive\n", info->name);
	free_mem(info);
	return 1;
}

int event_exit_handler (req_info_t *info)
{
	DBG ("%s%s has left the group%s\n", YELLOW, info->name, RESET);

	/* clear superpeer record if superpeer left */
	if (strncmp (info->peer, sp_info.sp_peer, sizeof (sp_info.sp_peer)) == 0) {
		memset (sp_info.sp_peer, '\0', sizeof (sp_info.sp_peer));
	}

	free_mem(info);
	return 1;
}

int event_join_handler (req_info_t *info)
{
	DBG ("%s%s: join %s group%s\n", LIGHT_BLUE, info->name, info->group, RESET);
	free_mem(info);
	return 1;
}

int event_leave_handler (req_info_t *info)
{
	DBG ("%s%s: leave %s group%s\n", LIGHT_BLUE, info->name, info->group, RESET);
	free_mem(info);
	return 1;
}

int event_whisper_handler (req_info_t *info)
{
	DBG ("%s%s: %s%s\n", RED, info->name, info->message, RESET);
	parse_whisper_message (info->node, info->message);
	free_mem(info);
	return 1;
}

int event_shout_handler (req_info_t *info)
{
	DBG ("%s%s: %s%s\n", LIGHT_GREEN, info->name, info->message, RESET);
	parse_shout_message (info->node, info->message);
	free_mem(info);
	return 1;
}

int process_event_msg (zmsg_t *msg, req_info_t *info)
{
	info->event 		= zmsg_popstr (msg);		// event
	info->peer 			= zmsg_popstr (msg);		// peer
	info->name 			= zmsg_popstr (msg);		// name

	if (strcmp(info->event, "WHISPER") != 0)
		info->group 	= zmsg_popstr (msg);		// group

	if (strcmp(info->event, "JOIN") != 0)
		info->message 	= zmsg_popstr (msg);		// message

	return 1;
}

/* helper functions */
static void store_conn_history (char *sh, char *uuid)
{
	struct sh_tbl record;
	memset(&record, 0, sizeof(struct sh_tbl));
	memcpy(record.short_hash, sh, SHORT_HASH_LENGTH);
	memcpy(record.uuid, uuid, SP_PEER_UUID_LENGTH);
	list_add(&record, true);

#ifdef __DEBUG__
	list_display();
#endif

	return;
}

static int is_peer_equal (char *u1, char *u2)
{
	if (strncmp (u1, u2, SP_PEER_UUID_LENGTH) == 0) {
		return 1;
	}
	return 0;
}

static int is_peer_online (zyre_t *node, char *uuid)
{
	zlist_t *plist = zyre_peers (node);
	int sz = (int) zlist_size (plist);

	if (sz == 0)	// have no peers online
		return 0;

	/* check if current online peers is match in list */
	while (zlist_next(plist) != NULL) {
		char *temp_peer = (char *) zlist_pop (plist);
		// DBG ("%s temp_peer\n", temp_peer);
		// DBG ("%s uuid\n", uuid);

		if (is_peer_equal (temp_peer, uuid) == 1) {
			// DBG ("%speer online: %s%s\n", LIGHT_BLUE, uuid, RESET);
			return 1;
		}
	}

	return 0;
}

static void parse_shout_message (zyre_t *node, char *message)
{
	if (strncmp (CMD_SP, message, strlen (CMD_SP)) == 0) {
		sprintf (sp_info.sp_peer, "%s", message + strlen(CMD_SP) + 1);
		DBG ("sp_peer: %s\n", sp_info.sp_peer);
	}
	return;
}

static void parse_whisper_message (zyre_t *node, char *message)
{
	if (strncmp (CMD_SSU, message, strlen (CMD_SSU)) == 0) {
		/*
			2. check sh(f) whether is in our table
			FORMAT: HEADER SHORTHASH SELFUUID
		*/
		bool in_list = false;
		char sz_header[strlen (CMD_SSU)];
		char sh[SHORT_HASH_LENGTH] = {0};
		char src_uuid[SP_PEER_UUID_LENGTH + 1] = {0};

		sscanf(message, "%s %s %s", sz_header, sh, src_uuid);
		// DBG ("header: %s, shorthash: %s, uuid: %s\n", sz_header, sh, src_uuid);

		struct sh_tbl *ptr = list_search_by_shorthash (sh, NULL);
		if (NULL == ptr) {
			DBG ("%sSearch [sh = %s] failed, no such element found%s\n", YELLOW, sh, RESET);
			store_conn_history (sh, src_uuid);
		} else {
			in_list = true;
			DBG ("%sSearch passed [sh = %s, uuid = %s]%s\n", YELLOW, sh, ptr->uuid, RESET);
		}

		if (node) {
			int peer_is_online = -1;
			char msg_to_send[MSG_TRANS_LENGTH] = {0};

			if (in_list) {
				peer_is_online = is_peer_online (node, ptr->uuid);
			}

			DBG ("%sin_list: %s, peer_is_online: %d%s\n", LIGHT_BLUE
			     , (in_list == true) ? "true" : "false"
			     , peer_is_online
			     , RESET);

			/*
				2-1. send back the uuid which have this shorthash if online
				FORMAT: HEADER-FOUND-UUID
			*/
			if (in_list == true && peer_is_online == 1) {
				DBG ("%speer online: %s%s\n", LIGHT_BLUE, ptr->uuid, RESET);
				sprintf (msg_to_send, "%s %d %s", CMD_SSU_RSP, SH_FOUND, ptr->uuid);
			}
			/*
				2-2. tell the p1 which sh not found or not in list
				FORMAT: HEADER-NOTFOUND
			*/
			else {

				DBG ("%speer offline: %s%s\n", LIGHT_BLUE, ptr->uuid, RESET);
				sprintf (msg_to_send, "%s %d", CMD_SSU_RSP, SH_NOT_FOUND);
			}

			// DBG ("%s[msg_to_send = %s] %s\n", LIGHT_GREEN, msg_to_send, RESET);
			// DBG ("%s[src_uuid = %s] %s\n", LIGHT_GREEN, src_uuid, RESET);

			src_uuid[SP_PEER_UUID_LENGTH] = '\0';
			msg_to_send[strlen(msg_to_send)] = '\0';
			send_whisper_msg (node, msg_to_send, src_uuid);
		} else {
			DBG ("node is not available\n");
		}
	} else if (strncmp (CMD_SSU_RSP, message, strlen (CMD_SSU_RSP)) == 0) {
		// RSPSSU 0
		// RSPSSU 1 78DB70F02D3D49412FE0031F3654BF05
		int sh_is_found;
		char sz_header[strlen (CMD_SSU_RSP)];
		char dest_uuid[SP_PEER_UUID_LENGTH + 1] = {0};

		sscanf(message, "%s %d", sz_header, &sh_is_found);

		if (sh_is_found == 1) {
			sscanf(message, "%*s %*d %s", dest_uuid);
		}

		DBG ("header: %s, sh_is_found: %d, dest_uuid: %s\n", sz_header, sh_is_found, dest_uuid);
	}
	return;
}