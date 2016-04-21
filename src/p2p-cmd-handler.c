#include <stdio.h>

#include "p2p-cmd-handler.h"

extern const char *HEADER_VALUE;
extern const char *P2P_GROUP_NAME;
extern sp_info_t sp_info;

bool terminated = false;
char sp_peer[32] = {0};

int process_terminate 	(zyre_t *node, zmsg_t *msg);
int process_shout 		(zyre_t *node, zmsg_t *msg);
int process_whisper 	(zyre_t *node, zmsg_t *msg);
int query_online_peers 	(zyre_t *node, zmsg_t *msg);
int sp_notify_peers		(zyre_t *node, zmsg_t *msg);
int process_set_sp 		(zyre_t *node, zmsg_t *msg);
int process_start 		(zyre_t *node, zmsg_t *msg);

user_cmd_table_t user_op_func_tbl[] = {
	{	"$TERM",		process_terminate 	},
	{	"SHOUT",		process_shout	 	},
	{	"WHISPER",		process_whisper	 	},
	{	"QUERY",		query_online_peers 	},
	{	"SETSP",		process_set_sp	 	},
	{	"START",		process_start	 	},
	{	"",				NULL	 			}
};

int process_terminate (zyre_t *node, zmsg_t *msg)
{
	terminated = true;
	return 1;
}

int process_shout (zyre_t *node, zmsg_t *msg)
{
	char *string = zmsg_popstr (msg);
	zyre_shouts (node, P2P_GROUP_NAME, "%s", string);

	if (string)
		free (string);
	return 1;
}

int process_whisper (zyre_t *node, zmsg_t *msg)
{
	char *peer 		= zmsg_popstr (msg);
	char *message 	= zmsg_popstr (msg);

	DBG("peer: %s, message: %s\n", peer, message);

	zyre_whispers  (node, peer, "%s", message);

	if (peer)		free(peer);
	if (message)	free(message);

	return 1;
}

int query_online_peers (zyre_t *node, zmsg_t *msg)
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
	DBG ("Own : %d\n", sp_info.own);
	
	return peers;
}

int process_set_sp (zyre_t *node, zmsg_t *msg)
{
	/* Set this header to indicate we're superpeer 
	   other peers will receive this header when join
	   the same group.
	*/
	if (sp_info.sp_peer[0] == '\0' && sp_info.own == 0) {
		zyre_set_header (node, "X-HEADER", HEADER_VALUE);
		DBG ("Set headers as %s\n", HEADER_VALUE);
		
		sp_info.own = 1;
	
		/* notify other peers I'm superpeer */
		zlist_t *list = zyre_peers (node);
		while  (zlist_next(list) != NULL) {
			char *online_peer = (char *) zlist_pop (list);
			DBG ("online_peer %s\n", online_peer);
			
			zmsg_t *lmsg = zmsg_new ();
			zmsg_pushstrf  	(lmsg, "%s-%s", SP_HEADER, zyre_uuid (node));
			zmsg_pushstr 	(lmsg, online_peer);
			process_whisper (node, lmsg);
			zmsg_destroy 	(&lmsg);

			if (online_peer)	free (online_peer);
		}
		zlist_destroy (&list);
	} else {
		DBG ("Do nothing...superpeer exists\n");
	}
	return 1;
}

int process_start (zyre_t *node, zmsg_t *msg)
{
	DBG ("\n");
	return 1;
}