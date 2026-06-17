/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parsing_args.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: noemi <noemi@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/15 14:50:30 by noemi             #+#    #+#             */
/*   Updated: 2026/06/10 19:33:05 by noemi            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "args.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

static int	is_positive_int(char *str)
{
	int	i;

	i = 0;
	if (!str || str[0] == '\0')
		return (0);
	while (str[i])
	{
		if (str[i] < '0' || str[i] > '9')
			return (0);
		i++;
	}
	if (strcmp(str, "0") == 0)
		return (0);
	return (1);
}

static int	check_scheduler(char *str, t_data *data)
{
	if (strcmp(str, "fifo") == 0)
		data->scheduler = FIFO;
	else if (strcmp(str, "edf") == 0)
		data->scheduler = EDF;
	else
		return (0);
	return (1);
}

int	parse_args(t_data *data, char **argv)
{
	if (!is_positive_int(argv[1]) || !is_positive_int(argv[2])
		|| !is_positive_int(argv[3]) || !is_positive_int(argv[4])
		|| !is_positive_int(argv[5]) || !is_positive_int(argv[6]))
	{
		fprintf(stderr, "Error: arguments must be positive integers\n");
		return (0);
	}
	if (!is_positive_int(argv[7]) && strcmp(argv[7], "0") != 0)
	{
		fprintf(stderr, "Error: dongle_cooldown must be >= 0\n");
		return (0);
	}
	data->number_of_coders = atoi(argv[1]);
	data->time_to_burnout = atoi(argv[2]);
	data->time_to_compile = atoi(argv[3]);
	data->time_to_debug = atoi(argv[4]);
	data->time_to_refactor = atoi(argv[5]);
	data->number_of_compiles_required = atoi(argv[6]);
	data->dongle_cooldown = atoi(argv[7]);
	if (!check_scheduler(argv[8], data))
	{
		fprintf(stderr, "Error: scheduler must be 'fifo' or 'edf'\n");
		return (0);
	}
	return (1);
}
