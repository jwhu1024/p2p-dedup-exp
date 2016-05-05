#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "p2p-cmd-handler.h"
#include "p2p-event-handler.h"
#include "p2p-common.h"
#include "p2p-list.h"

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
	DBG ("%sGot a whisper message%s\n", RED, RESET);
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
	return (strncmp (u1, u2, SP_PEER_UUID_LENGTH) == 0) ? 1 : 0;
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
		if (is_peer_equal (temp_peer, uuid) == 1) {
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

static int random_select_peer (zyre_t *node, char *p)
{
	time_t t;
	srand((unsigned) time(&t));

	// random select a peer to do dedup
	int peers = 0;
	zlist_t *plist = zyre_peers (node);
	int sz = (int) zlist_size (plist);

	int rand_num = rand() % sz;
	// DBG ("rand_num: %d\n", rand_num);

	if (sz > 0) {
		while (zlist_next(plist) != NULL) {
			if (peers == rand_num) {
				memcpy(p, (char *) zlist_pop (plist), SP_PEER_UUID_LENGTH);
				// DBG ("random select peer: %s\n", p);
				return 1;
			}
			peers++;
		}
	} else {
		// DBG ("No peers online, can't do dedup\n");
	}
	return 0;
}

static void prepare_command (JS_CMD_E jscmd, char *value, char *out)
{
	char cmd[1024] = {0};

	sprintf (cmd, "node ../js-bignum/oprf.js %d %s", (int) jscmd, value);
	p_run_command (cmd, out);
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
		bool is_dup = false;
		char sz_header[strlen (CMD_SSU)];
		char sh[SHORT_HASH_LENGTH] = {0};
		char filehash[SHA256_HASH_LENGTH] = {0};
		char src_uuid[SP_PEER_UUID_LENGTH] = {0};

		sscanf(message, "%s %s %s %s", sz_header, sh, filehash, src_uuid);
		DBG ("\n%s=== Received - \"%s\" from %s ===%s\n", LIGHT_PURPLE, message, src_uuid, RESET);

		struct sh_tbl *ptr = list_search_by_shorthash (sh, NULL);
		if (NULL == ptr) {
			DBG ("%sSearch [sh = %s] failed, no such element found%s\n", YELLOW, sh, RESET);
			// store_conn_history (sh, src_uuid);
		} else {
			if (strncmp (ptr->uuid, src_uuid, SP_PEER_UUID_LENGTH) != 0) {
				DBG ("%sSearch passed [sh = %s, uuid = %s]%s\n", YELLOW, sh, ptr->uuid, RESET);
			} else {
				DBG ("Discard this dedup if the pa is ourself\n");
				is_dup = true;
			}
			in_list = true;
		}

		if (node) {
			int peer_is_online = -1;
			char msg_to_send[MSG_TRANS_LENGTH] = {0};

			if (in_list == true && is_dup == false) {
				peer_is_online = is_peer_online (node, ptr->uuid);
			}

			DBG ("%sin_list: %s, peer_is_online: %d%s\n", LIGHT_BLUE
			     , (in_list == true) ? "true" : "false"
			     , peer_is_online
			     , RESET);

			/*
				2-1. send back the uuid which have this shorthash if online
				FORMAT: HEADER-FOUND-SH-UUID-FILEHASH
			*/
			if (in_list == true && peer_is_online == 1) {
				DBG ("%speer online: %s%s\n", LIGHT_BLUE, ptr->uuid, RESET);
				sprintf (msg_to_send, "%s %d %s %s %s", CMD_SSU_RSP, SH_FOUND, sh, ptr->uuid, filehash);
			}
			/*
				2-2. tell the p1 which sh not found or not in list
				FORMAT: HEADER-NOTFOUND-SH-FILEHASH
			*/
			else {
				DBG ("%speer offline: %s%s\n", LIGHT_BLUE, ptr->uuid, RESET);
				sprintf (msg_to_send, "%s %d %s %s", CMD_SSU_RSP, SH_NOT_FOUND, sh, filehash);
			}

			src_uuid[SP_PEER_UUID_LENGTH] = '\0';
			msg_to_send[strlen(msg_to_send)] = '\0';
			send_whisper_msg (node, msg_to_send, src_uuid);
		} else {
			DBG ("node is not available\n");
		}
	} else if (strncmp (CMD_SSU_RSP, message, strlen (CMD_SSU_RSP)) == 0) {
		// RSPSSU 0 010110001101 58dd5518cc2c29da0a650ffdb18605f5d7ae404b8d8304e4c6992e4df3addf24
		// RSPSSU SHISFOUND SHORTHASH FILEHASH
		// RSPSSU 1 010110001101 78DB70F02D3D49412FE0031F3654BF05 58dd5518cc2c29da0a650ffdb18605f5d7ae404b8d8304e4c6992e4df3addf24
		// RSPSSU SHISFOUND SHORTHASH DEST-UUID FILEHASH
		int sh_is_found;
		char sz_header[strlen (CMD_SSU_RSP)];
		char dest_uuid[SP_PEER_UUID_LENGTH] = {0};
		char sh[SHORT_HASH_LENGTH] = {0};
		char filehash[SHA256_HASH_LENGTH] = {0};
		char msg_to_send[MSG_TRANS_LENGTH] = {0};

		sscanf(message, "%s %d %s", sz_header, &sh_is_found, sh);

		if (sh_is_found == 1) {
			sscanf(message, "%*s %*d %*s %s %s", dest_uuid, filehash);
		} else {
			sscanf(message, "%*s %*d %*s %s", filehash);
			random_select_peer (node , dest_uuid);
		}

		DBG ("\n%s=== Received - %s %d %s %s %s ===%s\n", LIGHT_PURPLE, sz_header, sh_is_found, sh, dest_uuid, filehash, RESET);

		/*
			Send the selected peer uuid to SP
			SPREC SHORTHASH SELF-UUID
			SPREC 010110001101 78DB70F02D3D49412FE0031F3654BF05
		*/
		sprintf (msg_to_send, "%s %s %s", CMD_SP_REC, sh, zyre_uuid (node));
		msg_to_send[strlen(msg_to_send)] = '\0';
		send_whisper_msg (node, msg_to_send, sp_info.sp_peer);

		// OPRF begin... step 1
		char h1[OPRF_H1_LENGTH] = {0};
		prepare_command (DO_OPRF_H1, filehash, h1);

		memset (msg_to_send, '\0', sizeof(msg_to_send));
		sprintf (msg_to_send, "%s %s %s", CMD_SEND_OPRF_H1, h1, zyre_uuid (node));
		msg_to_send[strlen(msg_to_send)] = '\0';
		send_whisper_msg (node, msg_to_send, dest_uuid);
	} else if (strncmp (CMD_SP_REC, message, strlen (CMD_SP_REC)) == 0) {
		/*
			Received the selected peer uuid by client peer
			SPREC SHORTHASH SELF-UUID
			SPREC 010110001101 78DB70F02D3D49412FE0031F3654BF05
		*/
		char uuid[SP_PEER_UUID_LENGTH] = {0};
		char sh[SHORT_HASH_LENGTH] = {0};

		sscanf(message, "%*s %s %s", sh, uuid);

		store_conn_history (sh, uuid);
		DBG ("\n%s=== Received - %s %s %s ===%s\n", LIGHT_PURPLE, CMD_SP_REC, sh, uuid, RESET);
	} else if (strncmp (CMD_SEND_OPRF_H1, message, strlen (CMD_SEND_OPRF_H1)) == 0) {
		/*
			OPRFH1/OPRFK1 H1/K1 UUID
		*/
		char msg_to_send[MSG_TRANS_LENGTH] = {0};
		char uuid[SP_PEER_UUID_LENGTH] = {0};
		char h1[OPRF_H1_LENGTH] = {0};

		sscanf(message, "%*s %s %s", h1, uuid);

		DBG ("\n%s=== Received - %s %s %s ===%s\n", LIGHT_PURPLE, CMD_SEND_OPRF_H1, h1, uuid, RESET);

		// OPRF step 2
		char k1[OPRF_K1_LENGTH] = {0};
		prepare_command (DO_OPRF_K1, h1, k1);

		sprintf (msg_to_send, "%s %s %s", CMD_SEND_OPRF_K1, k1, zyre_uuid (node));
		msg_to_send[strlen(msg_to_send)] = '\0';
		send_whisper_msg (node, msg_to_send, uuid);
	} else if (strncmp (CMD_SEND_OPRF_K1, message, strlen (CMD_SEND_OPRF_K1)) == 0) {
		/*
			OPRFH1/OPRFK1 H1/K1 UUID
		*/
		char uuid[SP_PEER_UUID_LENGTH] = {0};
		char k1[OPRF_K1_LENGTH] = {0};

		sscanf(message, "%*s %s %s", k1, uuid);

		DBG ("\n%s=== Received - %s %s %s ===%s\n", LIGHT_PURPLE, CMD_SEND_OPRF_K1, k1, uuid, RESET);

		// OPRF step 3
		char koprf[OPRF_K1_LENGTH] = {0};
		prepare_command (DO_OPRF, k1, koprf);
	}
	return;
}