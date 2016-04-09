#include <stdio.h>

#include "p2p-utils.h"
#include "p2p-cmd-handler.h"

bool terminated = false;
extern const char *P2P_GROUP_NAME;

int process_terminate 	(zyre_t *node, zmsg_t *msg);
int process_shout 		(zyre_t *node, zmsg_t *msg);
int process_whisper 	(zyre_t *node, zmsg_t *msg);
int query_online_peers 	(zyre_t *node, zmsg_t *msg);

user_cmd_table_t user_op_func_tbl[] = {
	{	"$TERM",		process_terminate 	},
	{	"SHOUT",		process_shout	 	},
	{	"WHISPER",		process_whisper	 	},
	{	"QUERY",		query_online_peers 	},
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
	char *peer = zmsg_popstr (msg);
	char *message = zmsg_popstr (msg);

	DBG("peer: %s, message: %s\n", peer, message);

	zyre_whispers  (node, peer, "%s", message);

	if (peer)		free(peer);
	if (message)	free(message);

	return 1;
}

int query_online_peers (zyre_t *node, zmsg_t *msg)
{
	zlist_t *plist = zyre_peers (node);
	int sz = (int) zlist_size (plist);

	if (sz > 0) {
		while (zlist_next(plist) != NULL) {
			char *tmp = zlist_pop (plist);
			DBG ("peer %s\n", tmp);
		}	
	} else {
		DBG ("No peers online\n");
	}
	
	return 1;
}
