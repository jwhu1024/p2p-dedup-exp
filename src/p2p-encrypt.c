#include <stdio.h>
#include <openssl/sha.h>

#include "p2p-encrypt.h"

#define TMP_HASH_FILE		"/tmp/dedup-hash"

///////////////////////////////////////////////
// Usage
// unsigned char hash[SHA256_HASH_LENGTH] = {0};
// create_sha256_hash ("/vagrant/a", hash);
// dump_hash (hash, SHA256_HASH_LENGTH);

// unsigned char sha1hash[SHA1_HASH_LENGTH] = {0};
// create_sha1_hash (hash, sha1hash);
// dump_hash (sha1hash, SHA1_HASH_LENGTH);

// DBG ("hash[0] : %c\n", hash[0]);
// DBG ("hash[1] : %c\n", hash[1]);
// DBG ("hash[2] : %c\n", hash[2]);
///////////////////////////////////////////////

void create_sha1_hash (unsigned char *sha256_hash, unsigned char *outhash)
{
	char cmd[128] = {0};
	char value[256] = {0};

	_system ("echo %s > %s", sha256_hash, TMP_HASH_FILE);

	sprintf (cmd, "openssl dgst -sha1 %s", TMP_HASH_FILE);
	p_run_command (cmd, value);

	unsigned char *hash = (unsigned char *) strstr (value, " ");
	hash++;

	memcpy (outhash, hash, SHA1_HASH_LENGTH);

#if 1
	DBG ("cmd: %s\n", cmd);
	DBG ("value: %s\n", value);
	DBG ("outhash: %s\n", outhash);
#endif

	return;
}

void create_sha256_hash (char *filename, unsigned char *outhash)
{
	char cmd[128] = {0};
	char value[256] = {0};

	sprintf (cmd, "openssl dgst -sha256 %s", filename);

	p_run_command (cmd, value);

	unsigned char *hash = (unsigned char *) strstr (value, " ");
	hash++;

	memcpy (outhash, hash, SHA256_HASH_LENGTH);

#if 1
	DBG ("cmd: %s\n", cmd);
	DBG ("value: %s\n", value);
	DBG ("outhash: %s\n", outhash);
#endif

	return;
}

void dump_hash (unsigned char *hash, int len)
{
	int cl;
	DBG("hash key: ");
	for (cl = 0; cl < len; cl++)
		printf ("%c", hash[cl]);
	putchar ('\n');
	return;
}