#ifndef _P2P_UTILS_H_
#define _P2P_UTILS_H_

#include <stdint.h>
#include <string.h>

#include "zyre.h"
#include "czmq.h"

#define __SHORT_FILE__ ((strrchr(__FILE__, '/'))?  strrchr(__FILE__, '/') + 1 : __FILE__)

#ifdef __DEBUG__
#define DBG(message, ...) \
		printf("[%d][%s:%d:%s] " message, (int) time (NULL),	\
											__SHORT_FILE__,		\
											__LINE__, 			\
											__FUNCTION__, 		\
											##__VA_ARGS__)
#else
#define DBG(message, ...)
#endif

#define NONE 				"\033[m"
#define RED 				"\033[0;32;31m"
#define LIGHT_RED 			"\033[1;31m"
#define GREEN 				"\033[0;32;32m"
#define LIGHT_GREEN 		"\033[1;32m"
#define BLUE 				"\033[0;32;34m"
#define LIGHT_BLUE 			"\033[1;34m"
#define DARY_GRAY 			"\033[1;30m"
#define CYAN 				"\033[0;36m"
#define LIGHT_CYAN 			"\033[1;36m"
#define PURPLE 				"\033[0;35m"
#define LIGHT_PURPLE 		"\033[1;35m"
#define BROWN 				"\033[0;33m"
#define YELLOW 				"\033[1;33m"
#define LIGHT_GRAY 			"\033[0;37m"
#define WHITE 				"\033[1;37m"
#define RESET 				"\033[0m"

#define CMD_SP				"SP"		/* SetSP Header */
#define CMD_SSU				"SSU"		/* Send ShortHash UUID */
#define CMD_SSU_RSP			"RSPSSU"	/* Response */

#define SH_FOUND			1
#define SH_NOT_FOUND		0

#define SHA1_HASH_LENGTH	40
#define SHA256_HASH_LENGTH	64
#define SHORT_HASH_LENGTH	12
#define SP_PEER_UUID_LENGTH	32
#define MSG_TRANS_LENGTH	128

typedef struct {
	char sp_peer[SP_PEER_UUID_LENGTH];
	int own;
} sp_info_t;

void print_command_list (void);
int p_run_command (char *c, char *v);
int _system (const char *fmt, ...);
int is_valid_short_hash (char sh[]);

#endif /* _P2P_UTILS_H_ */