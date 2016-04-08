#include <stdio.h>

#include "p2p-utils.h"
#include "p2p-handler.h"

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
	free(info->event);
	free(info->peer);
	free(info->name);
	free(info->group);
	free(info->message);
}

int event_enter_handler (req_info_t *info)
{
	DBG ("%s%s has joined the chat%s\n", YELLOW, info->name, RESET);
	free_mem(info);
	return 1;
}

int event_evasive_handler (req_info_t *info)
{
	DBG ("%s is being evasive\n", info->name);
	return 1;
}

int event_exit_handler (req_info_t *info)
{
	DBG ("%s%s has left the chat%s\n", YELLOW, info->name, RESET);
	return 1;
}

int event_join_handler (req_info_t *info)
{
	return 1;
}

int event_leave_handler (req_info_t *info)
{
	return 1;
}

int event_whisper_handler (req_info_t *info)
{
	DBG ("%s%s: %s%s\n", LIGHT_GRAY, info->name, info->message, RESET);
	return 1;
}

int event_shout_handler (req_info_t *info)
{
	DBG ("%s%s: %s%s\n", LIGHT_GREEN, info->name, info->message, RESET);
	return 1;
}