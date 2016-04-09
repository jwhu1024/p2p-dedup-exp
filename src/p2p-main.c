#include <stdlib.h>
#include <time.h>

#include "p2p-handler.h"
#include "p2p-utils.h"
#include "zyre.h"

#define NODE_NAME_PREFIX	"cli-"
#define P2P_GROUP_NAME		"DEDUP"
#define LAN_IFACE			"eth1"
#define LAN_IFACE_PORT		5670
#define DISCOVER_INT		1000		/* milliseconds */


extern zyre_cmd_table_t zyre_op_func_tbl[];

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
	// zyre_print(node);
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

	bool terminated = false;

	zpoller_t *poller = zpoller_new (pipe, zyre_socket (node), NULL);

	while (!terminated) {
		void *which = zpoller_wait (poller, -1);

		if (which == pipe) {
			zmsg_t *msg = zmsg_recv (which);

			if (!msg)
				break;              //  Interrupted

			char *command = zmsg_popstr (msg);
			if (streq (command, "$TERM"))
				terminated = true;
			else if (streq (command, "SHOUT")) {
				char *string = zmsg_popstr (msg);
				zyre_shouts (node, "DEDUP", "%s", string);
			}
			else {
				puts ("E: invalid message to actor");
				assert (false);
			}
			free (command);
			zmsg_destroy (&msg);
		}
		else if (which == zyre_socket (node)) {
			zmsg_t *msg = zmsg_recv (which);

			req_info_t ri;
			memset(&ri, '\0', sizeof(req_info_t));

			ri.event 	= zmsg_popstr (msg);
			ri.peer 	= zmsg_popstr (msg);
			ri.name 	= zmsg_popstr (msg);
			ri.group 	= zmsg_popstr (msg);
			ri.message 	= zmsg_popstr (msg);

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
	char node_name[32] = {};
	gen_random_name(node_name);

	zactor_t *d_actor = zactor_new (dedup_actor, node_name);
	assert (d_actor);

	while (!zsys_interrupted) {
		char message [1024] = {0};
		if (!fgets (message, 1024, stdin))
			break;
		message [strlen (message) - 1] = 0;     // Drop the trailing linefeed
		zstr_sendx (d_actor, "SHOUT", message, NULL);
	}
	zactor_destroy (&d_actor);
	return 0;
}