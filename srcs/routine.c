/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   routine.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ayoufkir <ayoufkir@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/17 12:03:55 by ayoufkir          #+#    #+#             */
/*   Updated: 2026/09/22 12:22:31 by ayoufkir         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/codexion.h"

void	log_state(t_coder *coder, char *msg)
{
	t_simulation	*sim;

	sim = coder->sim;
	pthread_mutex_lock(&sim->print_mutex);
	if (!sim->print_stopped)
		printf("%ld %d %s\n", elapsed_ms(sim), coder->id, msg);
	pthread_mutex_unlock(&sim->print_mutex);
}

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
	int		stop;

	request_dongles(coder);
	queue_remove(&coder->sim->queue, coder);
	coder->deadline = elapsed_ms(coder->sim) + coder->sim->time_to_burnout;
	pthread_mutex_unlock(&coder->sim->queue.mutex);
	log_state(coder, "is compiling");
	usleep(coder->sim->time_to_compile * 1000);
	pthread_mutex_lock(&coder->sim->queue.mutex);
	release_dongles(coder);
	pthread_cond_broadcast(&coder->sim->queue.cond);
	stop = coder->sim->stop;
	pthread_mutex_unlock(&coder->sim->queue.mutex);
	return (stop);
}

int	finish_coding(t_coder *coder)
{
	int		stop;

	log_state(coder, "is debugging");
	usleep(coder->sim->time_to_debug * 1000);
	pthread_mutex_lock(&coder->sim->queue.mutex);
	stop = coder->sim->stop;
	pthread_mutex_unlock(&coder->sim->queue.mutex);
	if (stop)
		return (1);
	log_state(coder, "is refactoring");
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
