#ifndef _P2P_HANDLER_H_
#define _P2P_HANDLER_H_

#include "p2p-common.h"

typedef struct req_info {
	zyre_t *node;
	char *event;
	char *peer;
	char *name;
	char *group;
	char *message;
} req_info_t;

typedef struct zyre_cmd_table {
	char event[16];
	int (*EV_HANDLER) (req_info_t *info);
} zyre_cmd_table_t;

int process_event_msg (zmsg_t *msg, req_info_t *info);

#endif /* _P2P_HANDLER_H_ */