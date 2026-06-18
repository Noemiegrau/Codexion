/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   heap.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: noemi <noemi@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/12 14:13:43 by noemi             #+#    #+#             */
/*   Updated: 2026/06/18 14:30:11 by noemi            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "args.h"
#include <stdlib.h>

int	heap_init(t_heap *tab, int capacity)
{
	tab->nodes = malloc(sizeof(t_heap_node) * capacity);
	if (!tab->nodes)
		return (0);
	tab->size = 0;
	tab->capacity = capacity;
	return (1);
}


void	heap_push(t_heap *tab, int coder_id, long key)
{
	int	i;

	if (tab->size >= tab->capacity)
		return ;
	tab->nodes[tab->size].coder_id = coder_id;
	tab->nodes[tab->size].key = key;
	i = tab->size;
	tab->size++;
	while (i > 0
		&& sorting_keys(tab->nodes[i], tab->nodes[(i - 1) / 2]))
	{
		heap_swap(&tab->nodes[(i - 1) / 2], &tab->nodes[i]);
		i = (i - 1) / 2;
	}
}

t_heap_node	heap_pop(t_heap *tab)
{
	t_heap_node	min;

	if (tab->size == 0)
	{
		min.coder_id = -1;
		min.key = -1;
		return (min);
	}
	min = tab->nodes[0];
	tab->nodes[0] = tab->nodes[tab->size - 1];
	tab->size--;
	heap_pop_utils(tab);
	return (min);
}

t_heap_node	heap_peek(t_heap *tab)
{
	t_heap_node	empty;

	if (tab->size == 0)
	{
		empty.coder_id = -1;
		empty.key = -1;
		return (empty);
	}
	return (tab->nodes[0]);
}

void	free_heap(t_heap *tab)
{
	free(tab->nodes);
	tab->nodes = NULL;
	tab->size = 0;
	tab->capacity = 0;
}
