/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   routine.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ayoufkir <ayoufkir@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/17 12:03:55 by ayoufkir          #+#    #+#             */
/*   Updated: 2026/09/17 18:15:44 by ayoufkir         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	wait_for_dongles(t_coder *coder)
{
	pthread_mutex_lock(&coder->sim->queue.mutex);
	if (coder->sim->stop)
	{
		pthread_mutex_unlock(&coder->sim->queue.mutex);
		return (1);
	}
	coder->request_order = coder->sim->next_request_order;
	coder->sim->next_request_order++;
	queue_push(&coder->sim->queue, coder);
	while (!acquire_dongles(coder, coder->sim) && !coder->sim->stop)
		wait_short(&coder->sim->queue.cond, &coder->sim->queue.mutex);
	if (coder->sim->stop)
	{
		queue_remove(&coder->sim->queue, coder);
		pthread_mutex_unlock(&coder->sim->queue.mutex);
		return (1);
	}
	return (0);
}

int	compiling(t_coder *coder)
{
	long	now;
	int		stop;

	now = elapsed_ms(coder->sim);
	request_dongles(coder, now);
	queue_remove(&coder->sim->queue, coder);
	coder->deadline = elapsed_ms(coder->sim) + coder->sim->time_to_burnout;
	pthread_mutex_unlock(&coder->sim->queue.mutex);
	now = elapsed_ms(coder->sim);
	printf("%ld %d is compiling\n", now, coder->id);
	usleep(coder->sim->time_to_compile * 1000);
	pthread_mutex_lock(&coder->sim->queue.mutex);
	release_dongles(coder);
	pthread_cond_broadcast(&coder->sim->queue.cond);
	pthread_mutex_unlock(&coder->sim->queue.mutex);
	pthread_mutex_lock(&coder->sim->queue.mutex);
	stop = coder->sim->stop;
	pthread_mutex_unlock(&coder->sim->queue.mutex);
	return (stop);
}

int	finish_coding(t_coder *coder)
{
	long	now;
	int		stop;

	now = elapsed_ms(coder->sim);
	printf("%ld %d is debugging\n", now, coder->id);
	usleep(coder->sim->time_to_debug * 1000);
	pthread_mutex_lock(&coder->sim->queue.mutex);
	stop = coder->sim->stop;
	pthread_mutex_unlock(&coder->sim->queue.mutex);
	if (stop)
		return (1);
	now = elapsed_ms(coder->sim);
	printf("%ld %d is refactoring\n", now, coder->id);
	usleep(coder->sim->time_to_refactor * 1000);
	pthread_mutex_lock(&coder->sim->queue.mutex);
	stop = coder->sim->stop;
	pthread_mutex_unlock(&coder->sim->queue.mutex);
	if (stop)
		return (1);
	return (0);
}

void	*coder_routine(void *arg)
{
	int		i;
	t_coder	*coder;
	int		stop;

	i = 0;
	coder = (t_coder *)arg;
	while (i < coder->sim->number_of_compiles_required)
	{
		if (wait_for_dongles(coder))
			break ;
		stop = compiling(coder);
		if (stop)
			break ;
		stop = finish_coding(coder);
		if (stop)
			break ;
		i++;
		if (i == coder->sim->number_of_compiles_required)
		{
			pthread_mutex_lock(&coder->sim->queue.mutex);
			coder->done = 1;
			pthread_mutex_unlock(&coder->sim->queue.mutex);
		}
	}
	return (NULL);
}
