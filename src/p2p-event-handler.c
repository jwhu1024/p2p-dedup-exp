#include <stdio.h>

#include "p2p-event-handler.h"

extern sp_info_t sp_info;
extern const char *HEADER_VALUE;
extern const char *P2P_GROUP_NAME;

int event_enter_handler 	(req_info_t *info);
int event_evasive_handler 	(req_info_t *info);
int event_exit_handler 		(req_info_t *info);
int event_join_handler 		(req_info_t *info);
int event_leave_handler 	(req_info_t *info);
int event_whisper_handler 	(req_info_t *info);
int event_shout_handler 	(req_info_t *info);

zyre_cmd_table_t zyre_op_func_tbl[] = {
	{	"ENTER",		event_enter_handler 	},
	{	"EVASIVE",		event_evasive_handler 	},
	{	"EXIT",			event_exit_handler 		},
	{	"JOIN",			event_join_handler 		},
	{	"LEAVE",		event_leave_handler 	},
	{	"WHISPER",		event_whisper_handler 	},
	{	"SHOUT",		event_shout_handler 	},
	{	"",				NULL 					}
};

void free_mem (req_info_t *info)
{
	if (info->event)	free(info->event);
	if (info->peer)		free(info->peer);
	if (info->name)		free(info->name);
	if (info->group)	free(info->group);
	if (info->message)	free(info->message);
}

static void parse_whisper_message (char *message)
{
	if (strncmp (SP_HEADER, message, strlen(SP_HEADER)) == 0) {
		sprintf (sp_info.sp_peer, "%s", message+strlen(SP_HEADER)+1);
		DBG ("sp_peer: %s\n", sp_info.sp_peer);
	}
}

int event_enter_handler (req_info_t *info)
{
	DBG ("%s%s has joined the group%s\n", YELLOW, info->name, RESET);
	char *header = zyre_peer_header_value (info->node, info->peer, "X-HEADER");

	if (header != NULL) {
#if 0 //def __DEBUG__
		DBG ("%sNAME: %s%s\n", YELLOW, info->name, RESET);
		DBG ("%sPEER: %s%s\n", YELLOW, info->peer, RESET);
		DBG ("%sGROUP: %s%s\n", YELLOW, info->group, RESET);
		DBG ("%sMESSAGE: %s%s\n", YELLOW, info->message, RESET);
		DBG ("%sHEADER: %s%s\n", YELLOW, header, RESET);
#endif
		if (strcmp (header, HEADER_VALUE) == 0) {
			sprintf (sp_info.sp_peer, "%s", info->peer);
			DBG ("%s%s is a superpeer: %s%s\n", YELLOW, info->name, sp_info.sp_peer, RESET);
		}
	} else {
		DBG ("%sHEADER: NOT FOUND%s\n", YELLOW, RESET);
	}

	free_mem(info);
	return 1;
}

int event_evasive_handler (req_info_t *info)
{
	DBG ("%s is being evasive\n", info->name);
	free_mem(info);
	return 1;
}

int event_exit_handler (req_info_t *info)
{
	DBG ("%s%s has left the group%s\n", YELLOW, info->name, RESET);

	/* clear superpeer record if superpeer left */
	if (strncmp (info->peer, sp_info.sp_peer, sizeof (sp_info.sp_peer)) == 0) {
		memset (sp_info.sp_peer, '\0', sizeof (sp_info.sp_peer));
	}

	free_mem(info);
	return 1;
}

int event_join_handler (req_info_t *info)
{
	DBG ("%s%s: join %s group%s\n", LIGHT_BLUE, info->name, info->group, RESET);
	free_mem(info);
	return 1;
}

int event_leave_handler (req_info_t *info)
{
	DBG ("%s%s: leave %s group%s\n", LIGHT_BLUE, info->name, info->group, RESET);
	free_mem(info);
	return 1;
}

int event_whisper_handler (req_info_t *info)
{
	DBG ("%s%s: %s%s\n", RED, info->name, info->message, RESET);
	parse_whisper_message (info->message);
	free_mem(info);
	return 1;
}

int event_shout_handler (req_info_t *info)
{
	DBG ("%s%s: %s%s\n", LIGHT_GREEN, info->name, info->message, RESET);
	free_mem(info);
	return 1;
}

int process_event_msg (zmsg_t *msg, req_info_t *info)
{
	info->event 		= zmsg_popstr (msg);		// event
	info->peer 			= zmsg_popstr (msg);		// peer
	info->name 			= zmsg_popstr (msg);		// name

	if (strcmp(info->event, "WHISPER") != 0)
		info->group 	= zmsg_popstr (msg);		// group

	if (strcmp(info->event, "JOIN") != 0)
		info->message 	= zmsg_popstr (msg);		// message

	return 1;
}