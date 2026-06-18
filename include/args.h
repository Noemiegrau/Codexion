/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   args.h                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: noemi <noemi@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/15 14:49:28 by noemi             #+#    #+#             */
/*   Updated: 2026/06/18 16:57:57 by noemi            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef ARGS_H
# define ARGS_H

# include <pthread.h>

# define FIFO	0
# define EDF	1

typedef struct s_data
{
	int				number_of_coders;
	int				time_to_burnout;
	int				time_to_compile;
	int				time_to_debug;
	int				time_to_refactor;
	int				number_of_compiles_required;
	int				dongle_cooldown;
	int				scheduler;
}					t_data;

typedef struct s_heap_node
{
	int				coder_id;
	long			key;
}					t_heap_node;

typedef struct s_heap
{
	t_heap_node		*nodes;
	int				size;
	int				capacity;
}					t_heap;

typedef struct s_dongle_data
{
	int				dongle_id;
	pthread_mutex_t	mutex;
	pthread_cond_t	available;
	int				in_use;
	long			release_time;
	t_heap			wait_queue;
}					t_dongle_data;

typedef struct s_sim	t_sim;

typedef struct s_coder_data
{
	int				id_number;
	pthread_t		thread;
	t_dongle_data	*left_dongle;
	t_dongle_data	*right_dongle;
	long			last_compile;
	pthread_mutex_t	last_compile_mutex;
	int				compile_count;
	t_sim			*sim;
}					t_coder_data;

struct s_sim
{
	t_data			params;
	t_coder_data	*coders;
	t_dongle_data	*dongles;
	long			start_time;
	int				stop;
	pthread_mutex_t	stop_mutex;
	pthread_mutex_t	print_mutex;
	pthread_t		monitor_thread;
	long			seq;
	pthread_mutex_t	seq_mutex;
};

int			parse_args(t_data *data, char **argv);
int			init_sim(t_sim *sim);
void		*coder_routine(void *arg);
void		*monitor(void *arg);
int			check_all_compiled(t_sim *sim);
void		stop_simulation(t_sim *sim, int burned_id);

int			heap_init(t_heap *tab, int capacity);
void		heap_push(t_heap *tab, int coder_id, long key);
t_heap_node	heap_pop(t_heap *tab);
t_heap_node	heap_peek(t_heap *tab);
void		free_heap(t_heap *tab);
void		heap_swap(t_heap_node *a, t_heap_node *b);
int			sorting_keys(t_heap_node a, t_heap_node b);
void		heap_pop_utils(t_heap *tab);
long		get_priority(t_coder_data *coder);
void		release_dongle(t_coder_data *coder, t_dongle_data *dongle);
long		get_time_ms(t_sim *sim);
int			check_sim_state(t_sim *sim);
void		print_manager(t_sim *sim, int id, char *msg);

#endif
