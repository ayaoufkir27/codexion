/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   monitor.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ayoufkir <ayoufkir@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/17 13:25:46 by ayoufkir          #+#    #+#             */
/*   Updated: 2026/09/18 11:53:32 by ayoufkir         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/codexion.h"

void	track_burnout(t_simulation *sim, long now)
{
	int	i;

	i = 0;
	while (i < sim->number_of_coders)
	{
		if (now >= sim->coders[i].deadline && !sim->coders[i].done)
		{
			printf("%ld %d burned out\n", now, sim->coders[i].id);
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
