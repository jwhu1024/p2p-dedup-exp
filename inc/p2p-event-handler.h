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

typedef struct whisper_handler {
	char event[16];
	void (*WHISPER_HANDLER) (zyre_t *node, char *message);
} whisper_handler_t;

typedef struct oprf_info {
	char command[10];
	int sh_is_found;
	char k1[OPRF_K1_LENGTH];
	char h1[OPRF_H1_LENGTH];
	char koprf[OPRF_K1_LENGTH];
	char shorthash[SHORT_HASH_LENGTH];
	char filehash[SHA256_HASH_LENGTH];
	char dest_uuid[SP_PEER_UUID_LENGTH];
	char need_upload[8];
	char CF[1024];
	char CK[256];
} OPRF_S;

int process_event_msg (zmsg_t *msg, req_info_t *info);

#endif /* _P2P_HANDLER_H_ */