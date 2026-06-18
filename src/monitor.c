/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   monitor.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: noemi <noemi@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/13 19:00:00 by noemi             #+#    #+#             */
/*   Updated: 2026/06/18 17:04:15 by noemi            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "args.h"
#include <unistd.h>

static int	check_burnout(t_sim *sim)
{
	int		i;
	long	now;
	long	last;

	now = get_time_ms(sim);
	i = 0;
	while (i < sim->params.number_of_coders)
	{
		pthread_mutex_lock(&sim->coders[i].last_compile_mutex);
		last = sim->coders[i].last_compile;
		pthread_mutex_unlock(&sim->coders[i].last_compile_mutex);
		if (now - last > sim->params.time_to_burnout)
			return (sim->coders[i].id_number);
		i++;
	}
	return (0);
}

int	check_all_compiled(t_sim *sim)
{
	int	i;
	int	required;
	int	count;

	required = sim->params.number_of_compiles_required;
	i = 0;
	while (i < sim->params.number_of_coders)
	{
		pthread_mutex_lock(&sim->coders[i].last_compile_mutex);
		count = sim->coders[i].compile_count;
		pthread_mutex_unlock(&sim->coders[i].last_compile_mutex);
		if (count < required)
			return (0);
		i++;
	}
	return (1);
}

void	stop_simulation(t_sim *sim, int burned_id)
{
	int	i;

	if (burned_id)
		print_manager(sim, burned_id, "burned out");
	pthread_mutex_lock(&sim->stop_mutex);
	sim->stop = 1;
	pthread_mutex_unlock(&sim->stop_mutex);
	i = 0;
	while (i < sim->params.number_of_coders)
	{
		pthread_mutex_lock(&sim->dongles[i].mutex);
		pthread_cond_broadcast(&sim->dongles[i].available);
		pthread_mutex_unlock(&sim->dongles[i].mutex);
		i++;
	}
}

static void	wake_waiting_coders(t_sim *sim)
{
	int	i;

	i = 0;
	while (i < sim->params.number_of_coders)
	{
		pthread_mutex_lock(&sim->dongles[i].mutex);
		pthread_cond_broadcast(&sim->dongles[i].available);
		pthread_mutex_unlock(&sim->dongles[i].mutex);
		i++;
	}
}

void	*monitor(void *arg)
{
	t_sim	*sim;
	int		burned_id;

	sim = (t_sim *)arg;
	while (1)
	{
		usleep(1000);
		wake_waiting_coders(sim);
		burned_id = check_burnout(sim);
		if (burned_id)
		{
			stop_simulation(sim, burned_id);
			return (NULL);
		}
		if (check_all_compiled(sim))
		{
			stop_simulation(sim, 0);
			return (NULL);
		}
		if (check_sim_state(sim))
			return (NULL);
	}
	return (NULL);
}
