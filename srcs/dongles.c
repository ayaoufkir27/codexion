/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongles.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ayoufkir <ayoufkir@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/17 12:01:23 by ayoufkir          #+#    #+#             */
/*   Updated: 2026/09/25 11:21:23 by ayoufkir         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/codexion.h"

void	request_dongles(t_coder *coder)
{
	pthread_mutex_lock(&coder->left->mutex);
	coder->left->available = 0;
	pthread_mutex_unlock(&coder->left->mutex);
	log_state(coder, "has taken a dongle");
	pthread_mutex_lock(&coder->right->mutex);
	coder->right->available = 0;
	pthread_mutex_unlock(&coder->right->mutex);
	log_state(coder, "has taken a dongle");
}

void	release_dongles(t_coder *coder)
{
	long	now;

	now = elapsed_ms(coder->sim);
	pthread_mutex_lock(&coder->left->mutex);
	coder->left->available = 1;
	coder->left->free_at = coder->sim->dongle_cooldown + now;
	pthread_mutex_unlock(&coder->left->mutex);
	pthread_mutex_lock(&coder->right->mutex);
	coder->right->available = 1;
	coder->right->free_at = coder->sim->dongle_cooldown + now;
	pthread_mutex_unlock(&coder->right->mutex);
}

int	is_dongle_free(t_dongle *d, long now)
{
	int	free;

	pthread_mutex_lock(&d->mutex);
	free = d->available && now >= d->free_at;
	pthread_mutex_unlock(&d->mutex);
	return (free);
}

int	acquire_dongles(t_coder *coder, t_simulation *sim)
{
	int		i;
	t_coder	*other;
	long	now;

	i = 0;
	now = elapsed_ms(sim);
	if (coder->left == coder->right)
		return (0);
	if (!is_dongle_free(coder->left, now) || !is_dongle_free(coder->right, now))
		return (0);
	while (i < sim->queue.size)
	{
		other = sim->queue.heap[i];
		if (coder != other
			&& (coder->left == other->right
				|| coder->right == other->left)
			&& is_dongle_free(other->left, now)
			&& is_dongle_free(other->right, now)
			&& priority_queue(other, coder))
			return (0);
		i++;
	}
	return (1);
}
