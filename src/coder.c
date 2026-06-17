/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coder.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: noemi <noemi@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/13 19:00:00 by noemi             #+#    #+#             */
/*   Updated: 2026/06/17 15:28:50 by noemi            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "args.h"
#include <unistd.h>

/* retourne la cle de priorite selon le scheduler (FIFO=arrivee, EDF=deadline) */
static long	get_dongle_key(t_coder_data *coder)
{
	long	key;

	if (coder->sim->params.scheduler == FIFO)
	{
		pthread_mutex_lock(&coder->sim->seq_mutex);
		key = coder->sim->seq++;
		pthread_mutex_unlock(&coder->sim->seq_mutex);
	}
	else
		key = coder->last_compile
			+ coder->sim->params.time_to_burnout;
	return (key);
}

/* relache un dongle et reveille les coders en attente */
static void	release_dongle(t_coder_data *coder, t_dongle_data *dongle)
{
	pthread_mutex_lock(&dongle->mutex);
	dongle->in_use = 0;
	dongle->release_time = get_time_ms(coder->sim); // timestamp de liberation
	pthread_cond_broadcast(&dongle->available); // reveille tous les waiters
	pthread_mutex_unlock(&dongle->mutex);
}

/* attend son tour puis prend le dongle selon scheduler et cooldown */
static void	acquire_dongle(t_coder_data *coder, t_dongle_data *dongle)
{
	struct timespec	ts;
	int				cooldown;

	cooldown = coder->sim->params.dongle_cooldown;
	pthread_mutex_lock(&dongle->mutex);
	heap_push(&dongle->wait_queue, coder->id_number, get_dongle_key(coder));
	while (!coder->sim->stop && (dongle->in_use
		|| get_time_ms(coder->sim) < dongle->release_time + cooldown
		|| heap_peek(&dongle->wait_queue).coder_id != coder->id_number))
	{
		set_wait_ts(&ts);
		pthread_cond_timedwait(&dongle->available, &dongle->mutex, &ts);
	}
	if (!coder->sim->stop)
	{
		heap_pop(&dongle->wait_queue);
		dongle->in_use = 1;
	}
	pthread_mutex_unlock(&dongle->mutex);
	if (!coder->sim->stop)
		log_state(coder->sim, coder->id_number, "has taken a dongle");
}

/* un cycle complet : prend les 2 dongles, compile, debug, refactor */
static void	do_cycle(t_coder_data *coder, t_dongle_data *first,
	t_dongle_data *second)
{
	acquire_dongle(coder, first);
	if (coder->sim->stop)
		return ;
	acquire_dongle(coder, second);
	if (coder->sim->stop)
	{
		release_dongle(coder, first);
		return ;
	}
	pthread_mutex_lock(&coder->last_compile_mutex);
	coder->last_compile = get_time_ms(coder->sim); // reset deadline burnout
	pthread_mutex_unlock(&coder->last_compile_mutex);
	log_state(coder->sim, coder->id_number, "is compiling");
	usleep(coder->sim->params.time_to_compile * 1000);
	release_dongle(coder, first);
	release_dongle(coder, second);
	coder->compile_count++;
	if (coder->sim->stop)
		return ;
	log_state(coder->sim, coder->id_number, "is debugging");
	usleep(coder->sim->params.time_to_debug * 1000);
	if (coder->sim->stop)
		return ;
	log_state(coder->sim, coder->id_number, "is refactoring");
	usleep(coder->sim->params.time_to_refactor * 1000);
}

/* point d'entree du thread : boucle compile->debug->refactor */
void	*coder_routine(void *arg)
{
	t_coder_data	*coder;
	t_dongle_data	*first;
	t_dongle_data	*second;

	coder = (t_coder_data *)arg;
	if (coder->left_dongle == coder->right_dongle) // cas 1 coder : ne peut pas compiler
	{
		while (!coder->sim->stop)
			usleep(1000);
		return (NULL);
	}
	if (coder->left_dongle->dongle_id < coder->right_dongle->dongle_id)
	{
		first = coder->left_dongle; // toujours prendre le + petit id en premier
		second = coder->right_dongle; // prevention du deadlock
	}
	else
	{
		first = coder->right_dongle;
		second = coder->left_dongle;
	}
	while (!coder->sim->stop)
		do_cycle(coder, first, second);
	return (NULL);
}
