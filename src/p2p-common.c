#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "p2p-common.h"

int is_file_exist (const char *path)
{
	if ( access( path, F_OK ) != -1 )
		return 1;
	else
		return 0;
}

void print_command_list (void)
{
	printf("-----------------------------------------\n");
	printf("Command\t\tDescription\tDetail\n");
	printf("[1]\t\tSHOUT\t\tBroadcast to All Peers in Group\n");
	printf("[2]\t\tWHISPER\t\tSend Message to Single Peers\n");
	printf("[3]\t\tQUERY\t\tQuery Current Online Peers\n");
	printf("[4]\t\tSETSP\t\tSet Current Node as SuperPeer\n");
	printf("[5]\t\tSTART\t\tStart Test Process\n");
	printf("-----------------------------------------\n");
}

int _system (const char *fmt, ...)
{
	va_list args;
	int i;
	char buf[512];

	memset(buf, 0x0, 512);
	va_start(args, fmt);
	i = vsprintf(buf, fmt, args);
	va_end(args);

	system(buf);
	return i;
}

int p_run_command (char *c, char *v)
{
	FILE *fp = NULL;
	char buf[1024] = {0};

	memset (buf, 0x0, sizeof(buf));
	fp = popen (c, "r");
	fread(buf, sizeof(char), sizeof(buf), fp);
	pclose (fp);
	buf[strlen(buf) - 1] = '\0';
	strncpy(v, buf, strlen(buf) + 1);

	return strlen(v);
}

int is_valid_short_hash (char sh[])
{
	int i;
	int len = strlen (sh);

	if (len > 12) {
		return 0;
	}

	for (i = 0; i < len; ++i) {
		if (sh[i] != '0' && sh[i] != '1') {
			printf("sh[%d] %c not 0 or 1\n", i, sh[i]);
			return 0;
		}
	}

	return 1;
}