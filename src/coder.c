/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coder.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: noemi <noemi@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/13 19:00:00 by noemi             #+#    #+#             */
/*   Updated: 2026/06/18 17:18:44 by noemi            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "args.h"
#include <unistd.h>


static void	get_dongle(t_coder_data *coder, t_dongle_data *dongle)
{
	int	cooldown;

	cooldown = coder->sim->params.dongle_cooldown;
	pthread_mutex_lock(&dongle->mutex);
	heap_push(&dongle->wait_queue, coder->id_number, get_priority(coder));
	while (!check_sim_state(coder->sim) && (dongle->in_use
			|| get_time_ms(coder->sim) < dongle->release_time + cooldown
			|| heap_peek(&dongle->wait_queue).coder_id != coder->id_number))
	{
		pthread_cond_wait(&dongle->available, &dongle->mutex);
	}
	if (!check_sim_state(coder->sim))
	{
		heap_pop(&dongle->wait_queue);
		dongle->in_use = 1;
	}
	pthread_mutex_unlock(&dongle->mutex);
	if (!check_sim_state(coder->sim))
		print_manager(coder->sim, coder->id_number, "has taken a dongle");
}

static void	do_debug_refactor(t_coder_data *coder)
{
	if (check_sim_state(coder->sim))
		return ;
	print_manager(coder->sim, coder->id_number, "is debugging");
	usleep(coder->sim->params.time_to_debug * 1000);
	if (check_sim_state(coder->sim))
		return ;
	print_manager(coder->sim, coder->id_number, "is refactoring");
	usleep(coder->sim->params.time_to_refactor * 1000);
}

static void	start_routine(t_coder_data *coder, t_dongle_data *first,
	t_dongle_data *second)
{
	get_dongle(coder, first);
	if (check_sim_state(coder->sim))
		return ;
	get_dongle(coder, second);
	if (check_sim_state(coder->sim))
	{
		release_dongle(coder, first);
		return ;
	}
	pthread_mutex_lock(&coder->last_compile_mutex);
	coder->last_compile = get_time_ms(coder->sim);
	pthread_mutex_unlock(&coder->last_compile_mutex);
	print_manager(coder->sim, coder->id_number, "is compiling");
	usleep(coder->sim->params.time_to_compile * 1000);
	release_dongle(coder, first);
	release_dongle(coder, second);
	pthread_mutex_lock(&coder->last_compile_mutex);
	coder->compile_count++;
	pthread_mutex_unlock(&coder->last_compile_mutex);
	if (check_all_compiled(coder->sim))
		stop_simulation(coder->sim, 0);
	do_debug_refactor(coder);
}

void	*coder_routine(void *arg)
{
	t_coder_data	*coder;
	t_dongle_data	*first;
	t_dongle_data	*second;

	coder = (t_coder_data *)arg;
	if (coder->left_dongle == coder->right_dongle)
	{
		while (!check_sim_state(coder->sim))
			usleep(1000);
		return (NULL);
	}
	if (coder->left_dongle->dongle_id < coder->right_dongle->dongle_id)
	{
		first = coder->left_dongle;
		second = coder->right_dongle;
	}
	else
	{
		first = coder->right_dongle;
		second = coder->left_dongle;
	}
	while (!check_sim_state(coder->sim))
		start_routine(coder, first, second);
	return (NULL);
}
