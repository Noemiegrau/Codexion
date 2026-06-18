/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   heap_utils.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: noemi <noemi@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/18 17:19:36 by noemi             #+#    #+#             */
/*   Updated: 2026/06/18 17:19:37 by noemi            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "args.h"

void	heap_swap(t_heap_node *a, t_heap_node *b)
{
	t_heap_node	temp;

	temp = *a;
	*a = *b;
	*b = temp;
}

int	sorting_keys(t_heap_node a, t_heap_node b)
{
	if (a.key < b.key)
		return (1);
	return (a.key == b.key && a.coder_id < b.coder_id);
}

void	heap_pop_utils(t_heap *tab)
{
	int	i;
	int	smallest;
	int	left;
	int	right;

	i = 0;
	while (1)
	{
		left = 2 * i + 1;
		right = 2 * i + 2;
		smallest = i;
		if (left < tab->size
			&& sorting_keys(tab->nodes[left], tab->nodes[smallest]))
			smallest = left;
		if (right < tab->size
			&& sorting_keys(tab->nodes[right], tab->nodes[smallest]))
			smallest = right;
		if (smallest == i)
			break ;
		heap_swap(&tab->nodes[i], &tab->nodes[smallest]);
		i = smallest;
	}
}
