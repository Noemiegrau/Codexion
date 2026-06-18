/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coder_utils.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: noemi <noemi@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/18 17:00:00 by noemi             #+#    #+#             */
/*   Updated: 2026/06/18 17:00:00 by noemi            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "args.h"

long	get_priority(t_coder_data *coder)
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

void	release_dongle(t_coder_data *coder, t_dongle_data *dongle)
{
	pthread_mutex_lock(&dongle->mutex);
	dongle->in_use = 0;
	dongle->release_time = get_time_ms(coder->sim);
	pthread_cond_broadcast(&dongle->available);
	pthread_mutex_unlock(&dongle->mutex);
}
