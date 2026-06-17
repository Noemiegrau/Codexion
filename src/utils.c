/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: noemi <noemi@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/15 14:49:48 by noemi             #+#    #+#             */
/*   Updated: 2026/06/13 15:48:16 by noemi            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "args.h"
#include <sys/time.h>
#include <stdio.h>

long	get_time_ms(t_sim *sim)
{
	struct timeval	tv;

	gettimeofday(&tv, NULL);
	return (tv.tv_sec * 1000 + tv.tv_usec / 1000 - sim->start_time);
}

int	is_stopped(t_sim *sim)
{
	int	val;

	pthread_mutex_lock(&sim->stop_mutex);
	val = sim->stop;
	pthread_mutex_unlock(&sim->stop_mutex);
	return (val);
}

void	log_state(t_sim *sim, int id, char *msg)
{
	pthread_mutex_lock(&sim->print_mutex); // si un autre thread est entrain d'afficher, on attend la fin
	printf("%ld %d %s\n", get_time_ms(sim), id, msg);
	pthread_mutex_unlock(&sim->print_mutex);
}

/* remplit ts avec heure actuelle + 1ms pour pthread_cond_timedwait */
void	set_wait_ts(struct timespec *ts)
{
	struct timeval	tv;

	gettimeofday(&tv, NULL);
	ts->tv_sec = tv.tv_sec;
	ts->tv_nsec = (tv.tv_usec + 1000) * 1000;
	if (ts->tv_nsec >= 1000000000)
	{
		ts->tv_sec++;
		ts->tv_nsec -= 1000000000;
	}
}
