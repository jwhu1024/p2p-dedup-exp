#include <stdlib.h>
#include <time.h>

#include "p2p-cmd-handler.h"
#include "p2p-event-handler.h"
#include "p2p-common.h"
#include "p2p-encrypt.h"

#ifdef __DEBUG__
// #include "p2p-list.h"
#endif

#define NODE_NAME_PREFIX	"cli-"
#define LAN_IFACE			"eth1"
#define LAN_IFACE_PORT		5670
#define DISCOVER_INT		1000		/* milliseconds */
#define MAX_MSG_LEN			512

extern bool terminated;
extern zyre_cmd_table_t zyre_op_func_tbl[];
extern user_cmd_table_t user_op_func_tbl[];

const char *HEADER_VALUE 	= "SUPERNODE";
const char *P2P_GROUP_NAME 	= "DEDUP";

sp_info_t sp_info = {
	.sp_peer = "",
	.own = 0
};

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

// #ifdef __DEBUG__
// 	zyre_set_verbose (node);
// #endif

#ifdef __DEBUG__
	// zyre_print(node);
	DBG ("ZYRE_VERSION\t\t:%ld\n"	, zyre_version ());
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

#ifdef __DEBUG__
			zmsg_dump (msg);
#endif /* __DEBUG__ */

			ri.node = node;
			process_event_msg (msg, &ri);

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

#ifdef __DEBUG__
	// hash_test();
	// list_self_test  ();
#endif

	while (!zsys_interrupted) {
		char command [16] = {0};
		char message [MAX_MSG_LEN] = {0};

		print_command_list();

		if (!fgets (command, sizeof(command), stdin))
			break;

		if (strncmp(command, "1", strlen("1")) == 0) {
			if (!fgets (message, sizeof(message), stdin))
				break;

			message [strlen (message) - 1] = 0;
			zstr_sendx (d_actor, "SHOUT", message, NULL);
		}
		else if (strncmp(command, "2", strlen("2")) == 0) {
			char peer [32] = {0};

			if (!fgets (peer, sizeof(peer), stdin))
				break;

			if (!fgets (message, sizeof(message), stdin))
				break;

			message [strlen (message) - 1] = 0;
			peer [strlen (peer) - 1] = 0;
			zstr_sendx (d_actor, "WHISPER", message, peer, NULL);
		} else if (strncmp(command, "3", strlen("3")) == 0) {
			zstr_sendx (d_actor, "QUERY", "dummy", NULL);
		} else if (strncmp(command, "4", strlen("4")) == 0) {
			zstr_sendx (d_actor, "SETSP", "dummy", NULL);
		} else if (strncmp(command, "5", strlen("5")) == 0) {
			zstr_sendx (d_actor, "START", "dummy", NULL);
		} else {
			DBG ("Unknown command...\n");
		}
		memset(command, '\0', sizeof(command));
		memset(message, '\0', sizeof(message));
	}
	zactor_destroy (&d_actor);
	return 0;
}