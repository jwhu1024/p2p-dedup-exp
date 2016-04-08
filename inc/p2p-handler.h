#ifndef _P2P_HANDLER_H_
#define _P2P_HANDLER_H_

#include "czmq.h"
typedef struct req_info {
	char *event;
	char *peer;
	char *name;
	char *group;
	char *message;
} req_info_t;

typedef struct zyre_cmd_table {
	char cmd[16];
	int (*EV_HANDLER) (req_info_t *info);
} zyre_cmd_table_t;

#endif /* _P2P_HANDLER_H_ */