/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   args.h                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: noemi <noemi@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/15 14:49:28 by noemi             #+#    #+#             */
/*   Updated: 2026/06/06 15:19:41 by noemi            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef ARGS_H
# define ARGS_H

# include <pthread.h>

typedef struct s_data
{
	int				number_of_coders;
	int				time_to_burnout;
	int				time_to_compile;
	int				time_to_debug;
	int				time_to_refactor;
	int				number_of_compiles_required;
	int				dongle_cooldown;
	int				scheduler; // 0 = FIFO, 1 = EDF par exemple
}					t_data;

typedef struct s_dongle_data
{
	int					dongle_id;
	pthread_mutex_t		mutex; // verrou du dongle
	pthread_cond_t		available; // pour la file d'attente (FIFO/EDF)
	int					in_use; // 0 = libre, 1 = pris
}						t_dongle_data;

typedef struct s_coder_data
{
	int				id_number;
	pthread_t		thread; // identifiant du thread
	t_dongle_data	*left_dongle; // pointeur vers le dongle gauche
	t_dongle_data	*right_dongle; // pointeur vers le dongle droit
	long			last_compile; // timestamp du dernier compile
	int				compile_count; // nombre de compiles effectues
	t_data			*data; // acces aux parametres globaux
}					t_coder_data;

#endif