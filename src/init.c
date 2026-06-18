/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: noemi <noemi@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/13 15:00:00 by noemi             #+#    #+#             */
/*   Updated: 2026/06/13 16:05:19 by noemi            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "args.h"
#include <sys/time.h>
#include <stdlib.h>

static int	init_dongles(t_sim *sim)
{
	int	n;
	int	i;

	n = sim->params.number_of_coders;
	sim->dongles = malloc(sizeof(t_dongle_data) * n);
	if (!sim->dongles)
		return (0);
	i = 0;
	while (i < n)
	{
		sim->dongles[i].dongle_id = i;
		sim->dongles[i].in_use = 0;
		sim->dongles[i].release_time = -(long)sim->params.dongle_cooldown;
		pthread_mutex_init(&sim->dongles[i].mutex, NULL);
		pthread_cond_init(&sim->dongles[i].available, NULL);
		if (!heap_init(&sim->dongles[i].wait_queue, n))
			return (0);
		i++;
	}
	return (1);
}

static int	init_coders(t_sim *sim)
{
	int	n;
	int	i;

	n = sim->params.number_of_coders;
	sim->coders = malloc(sizeof(t_coder_data) * n);
	if (!sim->coders)
		return (0);
	i = 0;
	while (i < n)
	{
		sim->coders[i].id_number = i + 1;
		sim->coders[i].last_compile = 0;
		sim->coders[i].compile_count = 0;
		sim->coders[i].sim = sim;
		sim->coders[i].left_dongle = &sim->dongles[i];
		sim->coders[i].right_dongle = &sim->dongles[(i + 1) % n];
		pthread_mutex_init(&sim->coders[i].last_compile_mutex, NULL);
		i++;
	}
	return (1);
}

int	init_sim(t_sim *sim)
{
	struct timeval	tv;

	gettimeofday(&tv, NULL);
	sim->start_time = tv.tv_sec * 1000 + tv.tv_usec / 1000;
	sim->stop = 0;
	sim->seq = 0;
	sim->coders = NULL;
	sim->dongles = NULL;
	pthread_mutex_init(&sim->stop_mutex, NULL);
	pthread_mutex_init(&sim->print_mutex, NULL);
	pthread_mutex_init(&sim->seq_mutex, NULL);
	if (!init_dongles(sim) || !init_coders(sim))
		return (0);
	return (1);
}
