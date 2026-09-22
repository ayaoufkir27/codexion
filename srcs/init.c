/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ayoufkir <ayoufkir@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/17 12:03:08 by ayoufkir          #+#    #+#             */
/*   Updated: 2026/09/22 12:50:29 by ayoufkir         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/codexion.h"

int	init_coders(t_simulation *sim)
{
	int	num;
	int	i;

	num = sim->number_of_coders;
	sim->coders = malloc(sizeof(t_coder) * num);
	if (!sim->coders)
		return (1);
	i = 0;
	while (i < num)
	{
		sim->coders[i].id = i + 1;
		sim->coders[i].sim = sim;
		sim->coders[i].left = &sim->dongles[i];
		sim->coders[i].deadline = sim->time_to_burnout;
		sim->coders[i].done = 0;
		if (i == sim->number_of_coders - 1)
			sim->coders[i].right = &sim->dongles[0];
		else
			sim->coders[i].right = &sim->dongles[i + 1];
		i++;
	}
	return (0);
}

int	init_dongles(t_simulation *sim)
{
	int	num;
	int	i;

	num = sim->number_of_coders;
	i = 0;
	sim->dongles = malloc(sizeof(t_dongle) * num);
	if (!sim->dongles)
		return (1);
	while (i < num)
	{
		sim->dongles[i].id = i + 1;
		sim->dongles[i].available = 1;
		sim->dongles[i].free_at = 0;
		if (pthread_mutex_init(&sim->dongles[i].mutex, NULL) != 0)
		{
			sim->dongle_count = i;
			return (1);
		}
		i++;
	}
	sim->dongle_count = num;
	return (0);
}

int	init_queue(t_simulation *sim)
{
	sim->queue.capacity = sim->number_of_coders;
	sim->queue.size = 0;
	sim->queue.heap = malloc(sizeof(t_coder *) * sim->queue.capacity);
	if (!sim->queue.heap)
		return (1);
	if (pthread_mutex_init(&sim->queue.mutex, NULL) != 0)
		return (1);
	if (pthread_mutex_init(&sim->print_mutex, NULL) != 0)
		return (1);
	sim->print_stopped = 0;
	if (pthread_cond_init(&sim->queue.cond, NULL) != 0)
		return (1);
	return (0);
}

int	create_coders(t_simulation *sim)
{
	int	i;

	i = 0;
	while (i < sim->number_of_coders)
	{
		if (pthread_create(&sim->coders[i].thread,
				NULL, coder_routine, &sim->coders[i]) != 0)
			return (1);
		i++;
	}
	if (pthread_create(&sim->monitor, NULL, monitor_routine, sim) != 0)
		return (1);
	return (0);
}

void	join_coders(t_simulation *sim)
{
	int	i;

	i = 0;
	while (i < sim->number_of_coders)
	{
		pthread_join(sim->coders[i].thread, NULL);
		i++;
	}
	pthread_mutex_lock(&sim->queue.mutex);
	sim->stop = 1;
	pthread_mutex_unlock(&sim->queue.mutex);
	pthread_join(sim->monitor, NULL);
}
