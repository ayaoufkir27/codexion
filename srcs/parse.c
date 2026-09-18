/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parse.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ayoufkir <ayoufkir@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/17 12:03:55 by ayoufkir          #+#    #+#             */
/*   Updated: 2026/09/18 13:23:23 by ayoufkir         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/codexion.h"

int	is_integer(char *str)
{
	int	i;

	i = 0;
	if (!str[0])
		return (0);
	while (str[i])
	{
		if (!(str[i] >= '0' && str[i] <= '9'))
			return (0);
		i++;
	}
	return (1);
}

int	check_args(t_simulation *sim, char **av, int ac)
{
	int	i;

	i = 1;
	while (i < ac - 1)
	{
		if (!is_integer(av[i]))
		{
			printf("Invalid integer\n");
			return (-1);
		}
		if (atoi(av[i]) < 0)
		{
			printf("Arguments must be positive integers\n");
			return (-1);
		}
		i++;
	}
	sim->scheduler = av[8];
	if (strcmp(sim->scheduler, "fifo") != 0
		&& strcmp(sim->scheduler, "edf") != 0)
	{
		printf("Scheduler must be either fifo or edf\n");
		return (-1);
	}
	return (0);
}

int	parse_args(t_simulation *sim, char **av, int ac)
{
	if (ac != 9)
	{
		printf("Wrong number of arguments\n");
		return (-1);
	}
	if (check_args(sim, av, ac) == -1)
		return (-1);
	sim->number_of_coders = atoi(av[1]);
	sim->time_to_burnout = atoi(av[2]);
	sim->time_to_compile = atoi(av[3]);
	sim->time_to_debug = atoi(av[4]);
	sim->time_to_refactor = atoi(av[5]);
	sim->number_of_compiles_required = atoi(av[6]);
	sim->dongle_cooldown = atoi(av[7]);
	return (0);
}
