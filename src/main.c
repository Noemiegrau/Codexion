/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: noemi <noemi@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/15 14:49:37 by noemi             #+#    #+#             */
/*   Updated: 2026/06/17 15:32:42 by noemi            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "args.h"
#include <stdio.h>
#include <stdlib.h>

static int	start_threads(t_sim *sim)
{
	int	i;

	i = 0;
	if (pthread_create(&sim->monitor_thread, NULL, monitor, sim))
		return (0);
	while (i < sim->params.number_of_coders)
	{
		if (pthread_create(&sim->coders[i].thread, NULL,
				coder_routine, &sim->coders[i]))
			return (0);
		i++;
	}
	return (1);
}

static void	free_sim(t_sim *sim)
{
	int	i;
	int	n;

	n = sim->params.number_of_coders;
	pthread_join(sim->monitor_thread, NULL);
	i = 0;
	while (i < n)
	{
		pthread_join(sim->coders[i].thread, NULL);
		i++;
	}
	i = 0;
	while (i < n)
	{
		pthread_mutex_destroy(&sim->dongles[i].mutex);
		pthread_cond_destroy(&sim->dongles[i].available);
		free_heap(&sim->dongles[i].wait_queue);
		pthread_mutex_destroy(&sim->coders[i].last_compile_mutex);
		i++;
	}
	free(sim->dongles);
	free(sim->coders);
	pthread_mutex_destroy(&sim->stop_mutex);
	pthread_mutex_destroy(&sim->print_mutex);
	pthread_mutex_destroy(&sim->seq_mutex);
}

int	main(int argc, char **argv)
{
	t_sim	sim;

	if (argc != 9)
	{
		fprintf(stderr, "Error: wrong number of arguments\n");
		return (1);
	}
	if (!parse_args(&sim.params, argv))
		return (1);
	if (!init_sim(&sim))
	{
		fprintf(stderr, "Error: initialization failed\n");
		return (1);
	}
	if (!start_threads(&sim))
	{
		fprintf(stderr, "Error: thread creation failed\n");
		return (1);
	}
	free_sim(&sim);
	return (0);
}
