#include <stdio.h>
#include "p2p-encrypt.h"

#define TMP_HASH_FILE		"/tmp/dedup-hash"

static char * const quads[] = { "0000", "0001", "0010", "0011",
                                "0100", "0101", "0110", "0111",
                                "1000", "1001", "1010", "1011",
                                "1100", "1101", "1110", "1111",
                              };

// create sha1 hash based on hash of sha256
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

#if 0
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

#if 0
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

static char * hex_to_bin_quad (unsigned char c)
{
	if (c >= '0' && c <= '9') return quads[     c - '0'];
	if (c >= 'A' && c <= 'F') return quads[10 + c - 'A'];
	if (c >= 'a' && c <= 'f') return quads[10 + c - 'a'];
	return NULL;
}

void short_hash_calc (char *f, unsigned char short_hash[], unsigned char filehash[])
{
	// unsigned char hash[SHA256_HASH_LENGTH] = {0};
	create_sha256_hash (f, filehash);

	unsigned char sha1hash[SHA1_HASH_LENGTH] = {0};
	create_sha1_hash (filehash, sha1hash);

	int i;
	// char short_hash[SHORT_HASH_LENGTH * 4 + 1] = {0};
	memset (short_hash, '\0', SHORT_HASH_LENGTH);

	for (i = 0; i < SHORT_HASH_LENGTH/4; i++) {
		char *bstr = hex_to_bin_quad (filehash[i]);
		if (bstr) {
			strncat ((char *)short_hash, bstr, 4);
		}
	}
	
#ifdef __DEBUG__	
	// dump_hash (filehash, SHA256_HASH_LENGTH);
	// dump_hash (sha1hash, SHA1_HASH_LENGTH);
	// DBG ("short_hash : %s\n", short_hash);
	// DBG ("is_valid: %d\n", is_valid_short_hash ((char *) short_hash););
#endif

	return;
}

void hash_test (void)
{
	// Usage
	unsigned char hash[SHA256_HASH_LENGTH] = {0};
	create_sha256_hash ("/vagrant/a", hash);
	dump_hash (hash, SHA256_HASH_LENGTH);

	unsigned char sha1hash[SHA1_HASH_LENGTH] = {0};
	create_sha1_hash (hash, sha1hash);
	dump_hash (sha1hash, SHA1_HASH_LENGTH);

	// DBG ("hash[0] : %c\n", hash[0]);
	// DBG ("hash[1] : %c\n", hash[1]);
	// DBG ("hash[2] : %c\n", hash[2]);

	int i;
	char short_hash[SHORT_HASH_LENGTH] = {0};
	for (i = 0; i < SHORT_HASH_LENGTH/4; i++) {
		char *bstr = hex_to_bin_quad (hash[i]);
		if (bstr) {
			strncat (short_hash, bstr, 4);
		}
	}
	DBG ("short_hash : %s\n", short_hash);

	int is_valid = is_valid_short_hash (short_hash);
	DBG ("is_valid: %d\n", is_valid);
	return;
}