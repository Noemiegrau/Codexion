/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: noemi <noemi@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/15 14:49:37 by noemi             #+#    #+#             */
/*   Updated: 2026/05/16 20:07:26 by noemi            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include <unistd.h>
#include <stdio.h>
#include <pthread.h>

void	*coder_life(void *arg)
{
	// affiche "Coder X est en vie"
	printf("Coder X est en vie");
	// arg c'est le numero du coder, mais c'est un void*
	// comment tu le convertis en int ?
	int(arg);
	return (NULL);
}

int	main(int argc, char **argv)
{
	// int i;
	// char *s1;
	// char *s2;

	// i = 0;
	// s1 = argv[1];
	// s2 = argv[2];
	(void) argv;
	if (argc != 9)
	{
		write(1, "\n", 1);
		return (0);
	}

	// parsing_args() ?

	write(1, "This is a test.", 15);
	return (0);
}