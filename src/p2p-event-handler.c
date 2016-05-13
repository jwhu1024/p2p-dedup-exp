#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "p2p-cmd-handler.h"
#include "p2p-event-handler.h"
#include "p2p-encrypt.h"
#include "p2p-common.h"
#include "p2p-list.h"

extern sp_info_t sp_info;
extern const char *HEADER_VALUE;
extern const char *P2P_GROUP_NAME;
extern char g_filename[PATH_MAX];
static char priv_key[45] = {0};

struct sh_tbl *sh_table = NULL;
OPRF_S oprf_params;

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

static void cmd_ssu_op_func 				(zyre_t *node, char *message);
static void cmd_ssu_rsp_op_func 			(zyre_t *node, char *message);
static void cmd_sprec_op_func 				(zyre_t *node, char *message);
static void cmd_send_oprf_h1_op_func		(zyre_t *node, char *message);
static void cmd_send_oprf_k1_op_func 		(zyre_t *node, char *message);
static void cmd_send_koprf_op_func 			(zyre_t *node, char *message);
static void cmd_send_cfck_op_func 			(zyre_t *node, char *message);
static void cmd_list_forward_begin_op_func 	(zyre_t *node, char *message);
static void cmd_list_forward_op_func 		(zyre_t *node, char *message);
static void cmd_list_forward_end_op_func 	(zyre_t *node, char *message);

whisper_handler_t zyre_whisper_op_func_tbl[] = 	{
	{	CMD_SSU,			cmd_ssu_op_func 				},
	{	CMD_SSU_RSP,		cmd_ssu_rsp_op_func 			},
	{	CMD_SP_REC,			cmd_sprec_op_func 				},
	{	CMD_SEND_OPRF_H1,	cmd_send_oprf_h1_op_func 		},
	{	CMD_SEND_OPRF_K1,	cmd_send_oprf_k1_op_func 		},
	{	CMD_SEND_KOPRF,		cmd_send_koprf_op_func 			},
	{	CMD_SEND_CFCK,		cmd_send_cfck_op_func 			},
	{	CMD_LFWB,			cmd_list_forward_begin_op_func 	},
	{	CMD_LFW,			cmd_list_forward_op_func 		},
	{	CMD_LFWE,			cmd_list_forward_end_op_func 	},
	{	"",					NULL							}
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

	/* delete all about the exit peer's information when it leave group */
	list_delete_by_uuid (info->peer);
	list_display ();
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
	// DBG ("%sGot a whisper message%s\n", RED, RESET);
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

	if (list_update_by_shorthash (&record) == 0) {
		list_add(&record, true);
	}

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
	int peers = 0, sz = 0, rand_num = 0;
	char *self_uuid = (char *) zyre_uuid (node);
	zlist_t *plist;

REPEAT:
	// random select a peer to do dedup
	peers = 0;
	plist = zyre_peers (node);
	sz = (int) zlist_size (plist);

	rand_num = rand() % sz;

	if (sz > 0) {
		while (zlist_next(plist) != NULL) {
			char *tmp_uuid = (char *) zlist_pop (plist);

			if (peers == rand_num) {
				if (sz == 1) {
					memcpy(p, sp_info.sp_peer, SP_PEER_UUID_LENGTH);
					return 1;
				}

				if (strncmp (self_uuid, tmp_uuid, SP_PEER_UUID_LENGTH) == 0 		||
				        strncmp (sp_info.sp_peer, tmp_uuid, SP_PEER_UUID_LENGTH) == 0) 	{
					if ((sz / 2) > rand_num)	rand_num++;
					else					rand_num--;
					goto REPEAT;
				}

				memcpy(p, tmp_uuid, SP_PEER_UUID_LENGTH);
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

static void do_oprf_with_js (JS_CMD_E jscmd, char *value, char *out)
{
	char cmd[1024] = {0};

	sprintf (cmd, "node ../js/oprf.js %d %s", (int) jscmd, value);
	p_run_command (cmd, out);
	return;
}

static void get_params (OPRF_S *params, char *command, char *message)
{
	if (strncmp (CMD_SSU, command, strlen (CMD_SSU)) == 0) {
		sscanf(message, "%*s %s %s %s", params->shorthash
		       , params->filehash
		       , params->dest_uuid);
	} else if (strncmp (CMD_SSU_RSP, command, strlen (CMD_SSU_RSP)) == 0) {
		sscanf(message, "%*s %d %s", &params->sh_is_found
		       , params->shorthash);

		if (params->sh_is_found == 1) {
			sscanf(message, "%*s %*d %*s %s %s", params->dest_uuid, params->filehash);
		} else {
			sscanf(message, "%*s %*d %*s %s", params->filehash);
		}
	} else if (strncmp (CMD_SP_REC, command, strlen (CMD_SP_REC)) == 0) {
		sscanf(message, "%*s %s %s", params->shorthash
		       , params->dest_uuid);
	} else if (strncmp (CMD_SEND_OPRF_H1, command, strlen (CMD_SEND_OPRF_H1)) == 0) {
		sscanf(message, "%*s %s %s %s", params->h1
		       , params->filehash
		       , params->dest_uuid);
	} else if (strncmp (CMD_SEND_OPRF_K1, command, strlen (CMD_SEND_OPRF_K1)) == 0) {
		sscanf(message, "%*s %s %s %s", params->k1
		       , params->filehash
		       , params->dest_uuid);
	} else if (strncmp (CMD_SEND_KOPRF, command, strlen (CMD_SEND_KOPRF)) == 0) {
		sscanf(message, "%*s %s %s %s", params->koprf
		       , params->filehash
		       , params->dest_uuid);
	} else if (strncmp (CMD_SEND_CFCK, command, strlen (CMD_SEND_CFCK)) == 0) {
		sscanf(message, "%*s %s %s %s", params->CF
		       , params->CK
		       , params->filehash);
	}

	return;
}

static void gen_CFCK_with_js (JS_CMD_E jscmd, char *password, char *target, char *out)
{
	char cmd[1024] = {0};

	sprintf (cmd, "node ../js/oprf.js %d %s %s", (int) jscmd, password, target);
	p_run_command (cmd, out);
	return;
}

static void send_key_to_fakecloud (char *ser_ip, int ser_port, char *CK, char *out)
{
	char cmd[1024] = {0};

	_system ("rm -f /tmp/need_upload");
	sprintf (cmd, "http -b -f POST %s:%d key=\"%s\" > /tmp/need_upload && echo "" >> /tmp/need_upload", ser_ip, ser_port, CK);
	_system (cmd);

	sprintf (cmd, "cat /tmp/need_upload");
	p_run_command (cmd, out);
	return;
}

static void upload_file_to_fakecloud (char *ser_ip, int ser_port)
{
	_system ("http -f POST http://%s:%d/ file@%s", ser_ip, ser_port, g_filename);
	return;
}

static void cmd_ssu_op_func (zyre_t *node, char *message)
{
	char msg_to_send[MSG_TRANS_LENGTH] = {0};
	bool in_list = false;
	bool is_dup = false;

	memset (&oprf_params, '\0', sizeof (OPRF_S));
	get_params (&oprf_params, CMD_SSU, message);

	struct sh_tbl *ptr = list_search_by_shorthash (oprf_params.shorthash, NULL);
	if (NULL == ptr) {
		DBG ("%sSearch [sh = %s] failed, no such element found%s\n", YELLOW, oprf_params.shorthash, RESET);
	} else {
		if (strncmp (ptr->uuid, oprf_params.dest_uuid, SP_PEER_UUID_LENGTH) != 0) {
			DBG ("%sSearch passed [shorthash = %s, uuid = %s]%s\n", YELLOW, oprf_params.shorthash, ptr->uuid, RESET);
		} else {
			DBG ("Discard this dedup if the pa is ourself\n");
			is_dup = true;
		}
		in_list = true;
	}

	if (node) {
		int peer_is_online = -1;

		if (in_list == true && is_dup == false) {
			peer_is_online = is_peer_online (node, ptr->uuid);
		}

		DBG ("%sin_list: %s, peer_is_online: %d%s\n" , LIGHT_BLUE
		     , (in_list == true) ? "true" : "false"
		     , peer_is_online
		     , RESET);

		if (in_list == true && peer_is_online == 1) {
			DBG ("%speer online: %s%s\n", LIGHT_BLUE, ptr->uuid, RESET);
			sprintf (msg_to_send, "%s %d %s %s %s", CMD_SSU_RSP
			         , SH_FOUND
			         , oprf_params.shorthash
			         , ptr->uuid
			         , oprf_params.filehash);
		} else {
			// list_delete_by_shorthash (oprf_params.shorthash);
			DBG ("%speer offline: %s%s\n", LIGHT_BLUE, ptr->uuid, RESET);
			sprintf (msg_to_send, "%s %d %s %s", CMD_SSU_RSP
			         , SH_NOT_FOUND
			         , oprf_params.shorthash
			         , oprf_params.filehash);
		}

		oprf_params.dest_uuid[SP_PEER_UUID_LENGTH] = '\0';
		msg_to_send[strlen(msg_to_send)] = '\0';
		send_whisper_msg (node, msg_to_send, oprf_params.dest_uuid);
	} else {
		DBG ("node is not available\n");
	}
	return;
}

static void cmd_ssu_rsp_op_func (zyre_t *node, char *message)
{
	char msg_to_send[MSG_TRANS_LENGTH] = {0};

	memset (&oprf_params, '\0', sizeof (OPRF_S));
	get_params (&oprf_params, CMD_SSU_RSP, message);

	if (oprf_params.sh_is_found == 0) {
		random_select_peer (node , oprf_params.dest_uuid);
	}

	sprintf (msg_to_send, "%s %s %s", CMD_SP_REC
	         , oprf_params.shorthash
	         , oprf_params.dest_uuid);

	msg_to_send[strlen(msg_to_send)] = '\0';
	send_whisper_msg (node, msg_to_send, sp_info.sp_peer);

	// calculate h1
	do_oprf_with_js (DO_OPRF_H1, oprf_params.filehash, oprf_params.h1);

	memset (msg_to_send, '\0', sizeof(msg_to_send));
	sprintf (msg_to_send, "%s %s %s %s", CMD_SEND_OPRF_H1
	         , oprf_params.h1
	         , oprf_params.filehash
	         , zyre_uuid (node));

	msg_to_send[strlen(msg_to_send)] = '\0';
	send_whisper_msg (node, msg_to_send, oprf_params.dest_uuid);
}

static void cmd_sprec_op_func (zyre_t *node, char *message)
{
	memset (&oprf_params, '\0', sizeof (OPRF_S));
	get_params (&oprf_params, CMD_SP_REC, message);
	store_conn_history (oprf_params.shorthash, oprf_params.dest_uuid);
}

static void cmd_send_oprf_h1_op_func (zyre_t *node, char *message)
{
	char msg_to_send[MSG_TRANS_LENGTH] = {0};

	memset (&oprf_params, '\0', sizeof (OPRF_S));
	get_params (&oprf_params, CMD_SEND_OPRF_H1, message);

	// calculate k1
	do_oprf_with_js (DO_OPRF_K1, oprf_params.h1, oprf_params.k1);

	sprintf (msg_to_send, "%s %s %s %s"	, CMD_SEND_OPRF_K1
	         , oprf_params.k1
	         , oprf_params.filehash
	         , zyre_uuid (node));

	msg_to_send[strlen(msg_to_send)] = '\0';
	send_whisper_msg (node, msg_to_send, oprf_params.dest_uuid);
}

static void cmd_send_oprf_k1_op_func (zyre_t *node, char *message)
{
	char msg_to_send[MSG_TRANS_LENGTH] = {0};

	memset (&oprf_params, '\0', sizeof (OPRF_S));
	get_params (&oprf_params, CMD_SEND_OPRF_K1, message);

	// calculate OPRF's value
	do_oprf_with_js (DO_OPRF, oprf_params.k1, oprf_params.koprf);

	sprintf (msg_to_send, "%s %s %s %s"	, CMD_SEND_KOPRF
	         , oprf_params.koprf
	         , oprf_params.filehash
	         , zyre_uuid (node));

	msg_to_send[strlen(msg_to_send)] = '\0';
	send_whisper_msg (node, msg_to_send, oprf_params.dest_uuid);
	// DBG ("\n\nkoprf: %s\n", oprf_params.koprf);
}

static void cmd_send_koprf_op_func (zyre_t *node, char *message)
{
	char msg_to_send[MSG_TRANS_LENGTH] = {0};

	memset (&oprf_params, '\0', sizeof (OPRF_S));
	get_params (&oprf_params, CMD_SEND_KOPRF, message);

	if (priv_key[0] == '\0') {
		generate_random_key (priv_key);
	}

	gen_CFCK_with_js (DO_GEN_CF, priv_key, oprf_params.koprf, oprf_params.CF);
	gen_CFCK_with_js (DO_GEN_CK, priv_key, oprf_params.filehash, oprf_params.CK);

	sprintf (msg_to_send, "%s %s %s %s"	, CMD_SEND_CFCK
	         , oprf_params.CF
	         , oprf_params.CK
	         , oprf_params.filehash);

	msg_to_send[strlen(msg_to_send)] = '\0';
	send_whisper_msg (node, msg_to_send, oprf_params.dest_uuid);
}

static void cmd_send_cfck_op_func (zyre_t *node, char *message)
{
	memset (&oprf_params, '\0', sizeof (OPRF_S));
	get_params (&oprf_params, CMD_SEND_CFCK, message);

	// check the key with server
	send_key_to_fakecloud (SERVER_IP, SERVER_PORT, oprf_params.CF, oprf_params.need_upload);

	// check if we need to upload
	if (1 == atoi(oprf_params.need_upload)) {
		DBG ("Ready to upload file to server...\n");
		upload_file_to_fakecloud (SERVER_IP, SERVER_PORT);
	} else {
		DBG ("Don't need to upload the same file to server again...\n");
	}

	// DBG ("\n\nfilehash: %s\n", oprf_params.filehash);
	// DBG ("\n\nCF: %s\n", oprf_params.CF);
	// DBG ("\n\nCK: %s\n", oprf_params.CK);
	DBG ("%sneed_upload: %s%s\n", LIGHT_RED, oprf_params.need_upload, RESET);
}

static void cmd_list_forward_begin_op_func (zyre_t *node, char *message)
{
	// list_display (); // debug only
	return;
}

static void cmd_list_forward_op_func (zyre_t *node, char *message)
{
	struct sh_tbl stb;
	memset(&stb, 0, sizeof(struct sh_tbl));
	sscanf(message, "%*s %s %s", stb.uuid, stb.short_hash);
	list_add(&stb, true);
	return;
}

static void cmd_list_forward_end_op_func (zyre_t *node, char *message)
{
	list_display ();  // debug only
	force_set_sp_self (node);
	return;
}

static void parse_whisper_message (zyre_t *node, char *message)
{
	DBG ("\n%s=== Received - %s ===%s\n", LIGHT_GREEN, message, RESET);

	char command[16] = {0};
	sscanf (message, "%s", command);

	whisper_handler_t *wh = zyre_whisper_op_func_tbl;
	while (wh->event[0] != '\0') {
		if (streq (wh->event, command)) {
			wh->WHISPER_HANDLER(node, message);
			break;
		}
		wh++;
	}

	return;
}

void forward_list_to_next_sp (zyre_t *node)
{
	int n = 0;
	struct sh_tbl *s;

	list_display ();
	char msg_to_send[MSG_TRANS_LENGTH] = {0};

	strcpy (msg_to_send, CMD_LFWB);
	msg_to_send[strlen(msg_to_send)] = '\0';
	send_whisper_msg (node, msg_to_send, oprf_params.dest_uuid);

	if (list_count() > 0) {
		while ((s = list_get_by_index(n)) != NULL) {
			memset (msg_to_send, '\0', sizeof (msg_to_send));
			sprintf (msg_to_send, "%s %s %s", CMD_LFW
			         , s->uuid
			         , s->short_hash);

			msg_to_send[strlen(msg_to_send)] = '\0';
			send_whisper_msg (node, msg_to_send, oprf_params.dest_uuid);

			n++;
			zclock_sleep (300);		// wait for a while between two commands
		}

		memset (msg_to_send, '\0', sizeof (msg_to_send));
		strcpy (msg_to_send, CMD_LFWE);
		msg_to_send[strlen(msg_to_send)] = '\0';
		send_whisper_msg (node, msg_to_send, oprf_params.dest_uuid);
	}

	zclock_sleep (n * 500);			// wait for above commands all arrived
	return;
}