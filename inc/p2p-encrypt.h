#ifndef _P2P_ENCRYPT_H_
#define _P2P_ENCRYPT_H_

#include "p2p-common.h"

#define SHA1_HASH_LENGTH	40
#define SHA256_HASH_LENGTH	64
#define SHORT_HASH_LENGTH	3

void create_sha1_hash (unsigned char *sha256_hash, unsigned char *outhash);
void create_sha256_hash (char *filename, unsigned char *outhash);
void dump_hash (unsigned char *hash, int len);
void hash_test (void);

#endif /* _P2P_ENCRYPT_H_ */