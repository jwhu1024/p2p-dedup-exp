#include <stdio.h>
#include "p2p-common.h"

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
