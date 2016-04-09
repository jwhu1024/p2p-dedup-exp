#include <stdio.h>
#include "p2p-utils.h"

void print_command_list (void)
{
	printf("-----------------------------------------\n");
	printf("[QUERY]\t\tQuery Current Online Peers\n");
	printf("[SHOUT]\t\tBroadcast to All Peers in Group\n");
	printf("[WHISPER]\tSend Message to Single Peers\n");
	printf("-----------------------------------------\n");
}

int make_timer (timer_s_t *tst)
{
	struct sigevent         te;
	struct itimerspec       its;
	struct sigaction        sa;
	int                     sigNo = SIGRTMIN;

	/* Set up signal handler. */
	sa.sa_flags 	= SA_SIGINFO;
	sa.sa_sigaction = tst->timer_handler;
	sigemptyset(&sa.sa_mask);
	if (sigaction(sigNo, &sa, NULL) == -1) {
		fprintf(stderr, "%s: Failed to setup signal handling for .\n", tst->name);
		return (-1);
	}

	/* Set and enable alarm */
	te.sigev_notify = SIGEV_SIGNAL;
	te.sigev_signo 	= sigNo;
	te.sigev_value.sival_ptr = tst->timer_id;
	timer_create(CLOCK_REALTIME, &te, tst->timer_id);

	its.it_interval.tv_sec 	= tst->int_sec;
	its.it_interval.tv_nsec = 0;
	its.it_value.tv_sec 	= tst->exp_sec;
	its.it_value.tv_nsec 	= 0;
	timer_settime(*tst->timer_id, 0, &its, NULL);
	return (0);
}

void stop_timer (timer_t t_id)
{
	timer_delete(t_id);
}