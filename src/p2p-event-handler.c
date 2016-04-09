#include <stdio.h>

#include "p2p-utils.h"
#include "p2p-event-handler.h"

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

int event_enter_handler (req_info_t *info)
{
	DBG ("%s%s has joined the chat%s\n", YELLOW, info->name, RESET);
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
	DBG ("%s%s has left the chat%s\n", YELLOW, info->name, RESET);
	free_mem(info);
	return 1;
}

int event_join_handler (req_info_t *info)
{
	free_mem(info);
	return 1;
}

int event_leave_handler (req_info_t *info)
{
	free_mem(info);
	return 1;
}

int event_whisper_handler (req_info_t *info)
{
	DBG ("%s%s: %s%s\n", RED, info->name, info->message, RESET);
	free_mem(info);
	return 1;
}

int event_shout_handler (req_info_t *info)
{
	DBG ("%s%s: %s%s\n", LIGHT_GREEN, info->name, info->message, RESET);
	free_mem(info);
	return 1;
}