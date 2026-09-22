/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ayoufkir <ayoufkir@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/17 13:30:02 by ayoufkir          #+#    #+#             */
/*   Updated: 2026/09/20 20:00:04 by ayoufkir         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/codexion.h"

void	free_simulation(t_simulation *sim)
{
	int	i;

	if (sim->dongles)
	{
		i = 0;
		while (i < sim->dongle_count)
		{
			pthread_mutex_destroy(&sim->dongles[i].mutex);
			i++;
		}
		free(sim->dongles);
	}
	if (sim->coders)
		free(sim->coders);
	if (sim->queue.heap)
	{
		pthread_mutex_destroy(&sim->queue.mutex);
		pthread_cond_destroy(&sim->queue.cond);
		pthread_mutex_destroy(&sim->print_mutex);
		free(sim->queue.heap);
	}
}

long	get_time_ms(void)
{
	struct timeval	time;

	gettimeofday(&time, NULL);
	return (time.tv_sec * 1000L + time.tv_usec / 1000L);
}

long	elapsed_ms(t_simulation *sim)
{
	return (get_time_ms() - sim->start);
}

void	wait_short(pthread_cond_t *cond, pthread_mutex_t *mutex)
{
	struct timeval	now;
	struct timespec	ts;

	gettimeofday(&now, NULL);
	ts.tv_sec = now.tv_sec;
	ts.tv_nsec = (now.tv_usec + 5000) * 1000;
	if (ts.tv_nsec >= 1000000000)
	{
		ts.tv_sec += 1;
		ts.tv_nsec -= 1000000000;
	}
	pthread_cond_timedwait(cond, mutex, &ts);
}
