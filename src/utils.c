/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: noemi <noemi@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/15 14:49:48 by noemi             #+#    #+#             */
/*   Updated: 2026/06/18 17:17:49 by noemi            ###   ########.fr       */
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

int	check_sim_state(t_sim *sim)
{
	int	val;

	pthread_mutex_lock(&sim->stop_mutex);
	val = sim->stop;
	pthread_mutex_unlock(&sim->stop_mutex);
	return (val);
}

void	print_manager(t_sim *sim, int id, char *msg)
{
	pthread_mutex_lock(&sim->print_mutex);
	printf("%ld %d %s\n", get_time_ms(sim), id, msg);
	pthread_mutex_unlock(&sim->print_mutex);
}
