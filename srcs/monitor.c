/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   monitor.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ayoufkir <ayoufkir@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/17 13:25:46 by ayoufkir          #+#    #+#             */
/*   Updated: 2026/09/19 14:59:25 by ayoufkir         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/codexion.h"

void	log_burnout(t_simulation *sim, int id)
{
	pthread_mutex_lock(&sim->print_mutex);
	if (!sim->print_stopped)
		printf("%ld %d burned out\n", elapsed_ms(sim), id);
	sim->print_stopped = 1;
	pthread_mutex_unlock(&sim->print_mutex);
}

void	track_burnout(t_simulation *sim, long now)
{
	int	i;

	i = 0;
	while (i < sim->number_of_coders)
	{
		if (now >= sim->coders[i].deadline && !sim->coders[i].done)
		{
			log_burnout(sim, sim->coders[i].id);
			sim->stop = 1;
			pthread_cond_broadcast(&sim->queue.cond);
			break ;
		}
		i++;
	}
}

void	*monitor_routine(void *arg)
{
	t_simulation	*sim;
	int				stop;
	long			now;

	sim = (t_simulation *)arg;
	while (1)
	{
		pthread_mutex_lock(&sim->queue.mutex);
		stop = sim->stop;
		if (!stop)
		{
			now = elapsed_ms(sim);
			track_burnout(sim, now);
		}
		pthread_mutex_unlock(&sim->queue.mutex);
		if (stop)
			break ;
		usleep(1000);
	}
	return (NULL);
}
