#ifndef _P2P_CMD_HANDLER_H_
#define _P2P_CMD_HANDLER_H_

#include "p2p-common.h"

typedef struct node_info {
	zyre_t *node;
	zmsg_t *whole_msg;
	char *cmd;
	char *msg;
} node_info_t;

typedef struct user_cmd_table {
	char cmd[8];
	int (*CMD_HANDLER) (zyre_t *node, zmsg_t *msg);
} user_cmd_table_t;

void send_whisper_msg (zyre_t *node, char *msg, char *dest_peer);

#endif /* _P2P_CMD_HANDLER_H_ */