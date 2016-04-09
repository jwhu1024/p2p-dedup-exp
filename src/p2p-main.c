#include <stdlib.h>
#include <time.h>

#include "p2p-cmd-handler.h"
#include "p2p-event-handler.h"
#include "p2p-utils.h"

#define NODE_NAME_PREFIX	"cli-"
#define LAN_IFACE			"eth1"
#define LAN_IFACE_PORT		5670
#define DISCOVER_INT		1000		/* milliseconds */

#define _CMD_LINE_TEST_MODE_

extern bool terminated;
extern zyre_cmd_table_t zyre_op_func_tbl[];
extern user_cmd_table_t user_op_func_tbl[];

const char *P2P_GROUP_NAME = "DEDUP";

/*  This actor will listen and publish anything received
	on the DEDUP group.
*/
static void dedup_actor (zsock_t *pipe, void *args)
{
	zyre_t *node = zyre_new ((char *) args);
	if (!node) {
		DBG ("Could not create new node\n");
		return;
	}

	zyre_set_interface (node, LAN_IFACE);
	zyre_set_port (node, LAN_IFACE_PORT);
	zyre_set_interval (node, DISCOVER_INT);

#ifdef __DEBUG__
	zyre_set_verbose (node);
#endif

#ifdef __DEBUG__
	zyre_print(node);
	DBG ("ZYRE VERSION\t\t:%ld\n"	, zyre_version ());
	DBG ("NODE_NAME\t\t:%s\n"		, (char *) args);
	DBG ("LAN_IFACE\t\t:%s\n"		, LAN_IFACE);
	DBG ("LAN_IFACE_PORT\t\t:%d\n"	, LAN_IFACE_PORT);
	DBG ("DISCOVER_INT\t\t:%d\n"	, DISCOVER_INT);
	DBG ("--------------------------------\n");
#endif /* __DEBUG__ */

	if (zyre_start (node) == -1) {
		DBG ("Could not start a new node\n");
		return;
	}

	if (zyre_join (node, P2P_GROUP_NAME) == -1) {
		DBG ("Could not join to %s group\n", P2P_GROUP_NAME);
		return;
	}

	zsock_signal (pipe, 0);
	zpoller_t *poller = zpoller_new (pipe, zyre_socket (node), NULL);

	while (!terminated) {
		void *which = zpoller_wait (poller, -1);

		if (which == pipe) {
			zmsg_t *msg = zmsg_recv (which);
			if (!msg)
				break;

			char *command = zmsg_popstr(msg);

			user_cmd_table_t *p = user_op_func_tbl;
			while (p->cmd[0] != '\0') {
				if (strncmp (command, p->cmd, strlen(p->cmd)) == 0) {
					p->CMD_HANDLER(node, msg);
					break;
				}
				p++;
			}
			free (command);
			zmsg_destroy (&msg);
		}
		else if (which == zyre_socket (node)) {
			zmsg_t *msg = zmsg_recv (which);

			req_info_t ri;
			memset(&ri, '\0', sizeof(req_info_t));

			/* below should be fine tune by event */
			ri.event 	= zmsg_popstr (msg);
			ri.peer 	= zmsg_popstr (msg);
			ri.name 	= zmsg_popstr (msg);
			if (strcmp(ri.event, "SHOUT") == 0	||
				strcmp(ri.event, "JOIN") == 0)
				ri.group 	= zmsg_popstr (msg);
			ri.message 	= zmsg_popstr (msg);
			/* below should be fine tune by event */

			DBG("[EV] -------------------------\n");
			DBG("[EV] event : %s\n", ri.event);
			DBG("[EV] peer : %s\n", ri.peer);
			DBG("[EV] name : %s\n", ri.name);
			DBG("[EV] group : %s\n", ri.group);
			DBG("[EV] message : %s\n", ri.message);

			zyre_cmd_table_t *t = zyre_op_func_tbl;
			while (t->event[0] != '\0') {
				if (streq (ri.event, t->event)) {
					t->EV_HANDLER(&ri);
					break;
				}
				t++;
			}
			zmsg_destroy (&msg);
		}
	}

	zpoller_destroy (&poller);
	zyre_stop (node);
	zclock_sleep (100);
	zyre_destroy (&node);
}

int gen_random_name (char *name)
{
	srand(time(NULL));
	return sprintf(name, "%s%d", NODE_NAME_PREFIX, (rand() % 1000) + 1);;
}

int main (int argc, char *argv [])
{
#ifdef _CMD_LINE_TEST_MODE_
	char node_name[32] = {};
	gen_random_name(node_name);

	zactor_t *d_actor = zactor_new (dedup_actor, node_name);
	assert (d_actor);

	while (!zsys_interrupted) {
		char command [16] = {0};
		char message [512] = {0};

		print_command_list();

		if (!fgets (command, sizeof(command), stdin))
			break;

		if (strncmp(command, "SHOUT", strlen("SHOUT")) == 0) {
			if (!fgets (message, sizeof(message), stdin))
				break;

			message [strlen (message) - 1] = 0;     // Drop the trailing linefeed
			zstr_sendx (d_actor, command, message, NULL);
		}
		else if (strncmp(command, "WHISPER", strlen("WHISPER")) == 0) {
			char peer [32] = {0};

			if (!fgets (peer, sizeof(peer), stdin))
				break;

			if (!fgets (message, sizeof(message), stdin))
				break;

			// DBG ("[WHISPER] command : %s\n", command);
			// DBG ("[WHISPER] message : %s\n", message);
			// DBG ("[WHISPER] peer : %s\n", peer);

			message [strlen (message) - 1] = 0;
			peer [strlen (peer) - 1] = 0;
			zstr_sendx (d_actor, command, message, peer, NULL);
		} else if (strncmp(command, "QUERY", strlen("QUERY")) == 0) {
			zstr_sendx (d_actor, command, "dummy", NULL);
		} else {
			DBG ("Unknown command...\n");
		}
		memset(command, '\0', sizeof(command));
		memset(message, '\0', sizeof(message));
	}
	zactor_destroy (&d_actor);
	return 0;
#else

#endif
}
