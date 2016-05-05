#include <stdio.h>

#include "p2p-cmd-handler.h"
#include "p2p-encrypt.h"
#include "p2p-common.h"

extern const char *HEADER_VALUE;
extern const char *P2P_GROUP_NAME;
extern sp_info_t sp_info;
extern struct sh_tbl *sh_table;

bool terminated = false;

static int process_terminate 	(zyre_t *node, zmsg_t *msg);
static int process_shout 		(zyre_t *node, zmsg_t *msg);
static int process_whisper 		(zyre_t *node, zmsg_t *msg);
static int query_online_peers 	(zyre_t *node, zmsg_t *msg);
// static int sp_notify_peers		(zyre_t *node, zmsg_t *msg);
static int process_set_sp 		(zyre_t *node, zmsg_t *msg);
static int process_start 		(zyre_t *node, zmsg_t *msg);

user_cmd_table_t user_op_func_tbl[] = {
	{	"$TERM",		process_terminate 	},
	{	"SHOUT",		process_shout	 	},
	{	"WHISPER",		process_whisper	 	},
	{	"QUERY",		query_online_peers 	},
	{	"SETSP",		process_set_sp	 	},
	{	"START",		process_start	 	},
	{	"",				NULL	 			}
};

static int process_terminate (zyre_t *node, zmsg_t *msg)
{
	terminated = true;
	return 1;
}

static int process_shout (zyre_t *node, zmsg_t *msg)
{
	char *string = zmsg_popstr (msg);
	zyre_shouts (node, P2P_GROUP_NAME, "%s", string);

	if (string)
		free (string);
	return 1;
}

static int process_whisper (zyre_t *node, zmsg_t *msg)
{
	char *peer 		= zmsg_popstr (msg);
	char *message 	= zmsg_popstr (msg);

	// DBG("peer: %s, message: %s\n", peer, message);

	zyre_whispers  (node, peer, "%s", message);

	if (peer)		free(peer);
	if (message)	free(message);

	return 1;
}

static int query_online_peers (zyre_t *node, zmsg_t *msg)
{
	int peers = 0;
	zlist_t *plist = zyre_peers (node);
	int sz = (int) zlist_size (plist);

	if (sz > 0) {
#ifdef __DEBUG__
		while (zlist_next(plist) != NULL) {
			DBG ("peer %s\n", (char *) zlist_pop (plist));
			peers++;
		}
#endif
	} else {
		DBG ("No peers online\n");
	}

	DBG ("SuperPeer : %s\n", sp_info.sp_peer);
	DBG ("Self : %s\n", zyre_uuid (node));
	DBG ("Own : %d\n", sp_info.own);

	return peers;
}

static int process_set_sp (zyre_t *node, zmsg_t *msg)
{
	sp_info_t *sp_p = &sp_info;

	/* Set this header to indicate we're superpeer
	   other peers will receive this header when join
	   the same group.
	*/
	if (sp_p->sp_peer[0] == '\0' && sp_p->own == 0) {
		zyre_set_header (node, "X-HEADER", HEADER_VALUE);
		DBG ("Set headers as %s\n", HEADER_VALUE);

		snprintf (sp_p->sp_peer, SP_PEER_UUID_LENGTH + 1, "%s", zyre_uuid (node));
		sp_p->own = 1;

		/* notify other peers I'm superpeer by using broadcast*/
		char msg_to_send[MSG_TRANS_LENGTH] = {0};
		sprintf (msg_to_send, "%s-%s", CMD_SP, zyre_uuid (node));
		send_shout_msg (node, msg_to_send);
	} else {
		DBG ("Do nothing...superpeer exists\n");
	}
	return 1;
}

static int process_start (zyre_t *node, zmsg_t *msg)
{
	sp_info_t *sp_p = &sp_info;

	DBG ("%s===== Start our dedup process =====%s\n", LIGHT_PURPLE, RESET);

	// clear some temp files
	char out[16];
	p_run_command ("rm /tmp/n", out);
	p_run_command ("rm /tmp/r", out);

	if (sp_p->sp_peer[0] == '\0') {
		DBG ("%sCan't do OPRF before we known who is the sp, discard it! %s\n", LIGHT_RED, RESET);
		return 0;
	}

	if (strncmp (sp_p->sp_peer, zyre_uuid (node), SP_PEER_UUID_LENGTH) == 0) {
		DBG ("%sCan't do OPRF with ourself, discard it!%s\n", LIGHT_RED, RESET);
		return 0;
	}

	unsigned char short_hash[SHORT_HASH_LENGTH];
	unsigned char filehash[SHA256_HASH_LENGTH];
	short_hash_calc ("/vagrant/a", short_hash, filehash);

	char msg_to_send[MSG_TRANS_LENGTH] = {0};

	/* SSU SHORTHASH FILEHASH UUID */
	sprintf (msg_to_send, "%s %s %s %s", CMD_SSU, short_hash, filehash, zyre_uuid (node));
	send_whisper_msg (node, msg_to_send, sp_p->sp_peer);
	return 1;
}

/* helper functions */
void send_whisper_msg (zyre_t *node, char *msg, char *dest_peer)
{
	zmsg_t *lmsg = zmsg_new ();
	zmsg_pushstrf  	(lmsg, "%s", msg);
	zmsg_pushstr 	(lmsg, dest_peer);
	process_whisper (node, lmsg);
	zmsg_destroy 	(&lmsg);

	DBG ("\n%s=== Send \"%s\" to %s ===%s\n", LIGHT_PURPLE, msg, dest_peer, RESET);
	return;
}

void send_shout_msg (zyre_t *node, char *msg)
{
	zmsg_t *lmsg = zmsg_new ();
	zmsg_pushstrf  	(lmsg, "%s", msg);
	process_shout (node, lmsg);
	zmsg_destroy 	(&lmsg);

	DBG ("%s=== Send \"%s\" to all peers ===%s\n", LIGHT_PURPLE, msg, RESET);
	return;
}
