/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ayoufkir <ayoufkir@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/17 12:18:34 by ayoufkir          #+#    #+#             */
/*   Updated: 2026/09/18 11:53:28 by ayoufkir         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/codexion.h"

int	start_simulation(t_simulation *sim, int ac, char **av)
{
	sim->stop = 0;
	sim->next_request_order = 1;
	if (parse_args(sim, av, ac) == -1)
		return (1);
	if (init_dongles(sim))
		return (1);
	if (init_coders(sim))
	{
		free_simulation(sim);
		return (1);
	}
	if (init_queue(sim))
	{
		free_simulation(sim);
		return (1);
	}
	return (0);
}

int	main(int ac, char **av)
{
	t_simulation	sim;

	if (start_simulation(&sim, ac, av))
		return (1);
	sim.start = get_time_ms();
	if (create_coders(&sim))
	{
		free_simulation(&sim);
		return (1);
	}
	join_coders(&sim);
	free_simulation(&sim);
	printf("program finished lol\n");
	return (0);
}
