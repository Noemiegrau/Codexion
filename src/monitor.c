/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   monitor.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: noemi <noemi@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/13 19:00:00 by noemi             #+#    #+#             */
/*   Updated: 2026/06/13 19:00:00 by noemi            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "args.h"
#include <unistd.h>

/* verifie si un coder a depasse son temps limite - retourne son id ou 0 */
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

/* retourne 1 si tous les coders ont compile assez de fois */
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

/* log burnout si besoin, set stop=1, reveille tous les threads en attente */
void	stop_simulation(t_sim *sim, int burned_id)
{
	int	i;

	if (burned_id)
		log_state(sim, burned_id, "burned out"); // log dans les 10ms
	pthread_mutex_lock(&sim->stop_mutex);
	sim->stop = 1;
	pthread_mutex_unlock(&sim->stop_mutex);
	i = 0;
	while (i < sim->params.number_of_coders)
	{
		pthread_mutex_lock(&sim->dongles[i].mutex);
		pthread_cond_broadcast(&sim->dongles[i].available); // debloquer les waiters
		pthread_mutex_unlock(&sim->dongles[i].mutex);
		i++;
	}
}

/* reveille tous les coders en attente sur un dongle */
static void	broadcast_all_dongles(t_sim *sim)
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

/* thread monitor : detecte burnout et fin de simulation toutes les 1ms */
void	*monitor(void *arg)
{
	t_sim	*sim;
	int		burned_id;

	sim = (t_sim *)arg;
	while (1)
	{
		usleep(1000);
		broadcast_all_dongles(sim);
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
		if (is_stopped(sim))
			return (NULL);
	}
	return (NULL);
}
