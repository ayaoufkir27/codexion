/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ayoufkir <ayoufkir@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/17 13:30:02 by ayoufkir          #+#    #+#             */
/*   Updated: 2026/09/17 13:33:56 by ayoufkir         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void	free_simulation(t_simulation *sim)
{
	int	i;

	i = 0;
	while (i < sim->number_of_coders)
	{
		pthread_mutex_destroy(&sim->dongles[i].mutex);
		i++;
	}
	pthread_mutex_destroy(&sim->queue.mutex);
	pthread_cond_destroy(&sim->queue.cond);
	free(sim->queue.heap);
	free(sim->coders);
	free(sim->dongles);
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
	struct timespec	ts;
	struct timeval	now;

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
