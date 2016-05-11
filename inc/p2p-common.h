#ifndef _P2P_UTILS_H_
#define _P2P_UTILS_H_

#include <stdint.h>
#include <string.h>
#include <linux/limits.h>

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

#define SERVER_IP			"192.168.1.112"
#define SERVER_PORT			3000

#define CMD_SP				"SP"		/* SetSP Header */

/*
SSU 010110001101 026a333f8d5ac0754cbcb24de92787693eaf0a527c22d16739499617388dc459 78DB70F02D3D49412FE0031F3654BF05
*/
#define CMD_SSU				"SSU"		/* Send ShortHash UUID 			*/

/*
RSPSSU 1 010110001101 78DB70F02D3D49412FE0031F3654BF05 026a333f8d5ac0754cbcb24de92787693eaf0a527c22d16739499617388dc459
RSPSSU 0 010110001101 026a333f8d5ac0754cbcb24de92787693eaf0a527c22d16739499617388dc459
*/
#define CMD_SSU_RSP			"RSPSSU"	/* Response 					*/

/*
SPREC 010110001101 78DB70F02D3D49412FE0031F3654BF05
*/
#define CMD_SP_REC			"SPREC"		/* Send UUID to SP for record 	*/

/*
OPRFH1 H1 FILEHASH OURUUID
*/
#define CMD_SEND_OPRF_H1	"OPRFH1"	/* Send H1 of OPRF to pa		*/

/*
OPRFK1 K1 FILEHASH OURUUID
*/
#define CMD_SEND_OPRF_K1	"OPRFK1"	/* Send K1 of OPRF to p1 		*/

#define SH_FOUND			1
#define SH_NOT_FOUND		0

#define HEADER_MAX_LEN		10
#define SHA1_HASH_LENGTH	40
#define SHA256_HASH_LENGTH	64+1
#define SHORT_HASH_LENGTH	12+1
#define SP_PEER_UUID_LENGTH	32+1
#define MSG_TRANS_LENGTH	1024
#define OPRF_H1_LENGTH		1024
#define OPRF_K1_LENGTH		OPRF_H1_LENGTH

typedef struct {
	char sp_peer[SP_PEER_UUID_LENGTH];
	int own;
} sp_info_t;

typedef enum {
	DO_OPRF_H1 = 1,
	DO_OPRF_K1,
	DO_OPRF,
	DO_GEN_CF,
	DO_GEN_CK
} JS_CMD_E;

typedef enum {
	SEND_KEY = 1,
	DO_UPLOAD
} JS_CMD2_E;

void print_command_list (void);
int p_run_command (char *c, char *v);
int _system (const char *fmt, ...);
int is_valid_short_hash (char sh[]);
int is_file_exist(const char *path);

#endif /* _P2P_UTILS_H_ */