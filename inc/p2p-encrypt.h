#ifndef _P2P_ENCRYPT_H_
#define _P2P_ENCRYPT_H_

#include "p2p-common.h"

void create_sha1_hash (unsigned char *sha256_hash, unsigned char *outhash);
void create_sha256_hash (char *filename, unsigned char *outhash);
void dump_hash (unsigned char *hash, int len);
void hash_test (void);
void short_hash_calc (char *f, unsigned char short_hash[]);

#endif /* _P2P_ENCRYPT_H_ */