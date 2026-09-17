/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   heap.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ayoufkir <ayoufkir@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/17 12:20:16 by ayoufkir          #+#    #+#             */
/*   Updated: 2026/09/17 16:09:03 by ayoufkir         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	priority_queue(t_coder *a, t_coder *b)
{
	if (strcmp(a->sim->scheduler, "edf") == 0)
	{
		if (a->deadline != b->deadline)
			return (a->deadline < b->deadline);
		return (a->request_order < b->request_order);
	}
	return (a->request_order < b->request_order);
}

void	queue_push(t_queue *queue, t_coder *coder)
{
	int		i;
	int		parent;
	t_coder	*tmp;

	i = queue->size;
	queue->heap[i] = coder;
	queue->size++;
	while (i > 0)
	{
		parent = (i - 1) / 2;
		if (!priority_queue(queue->heap[i], queue->heap[parent]))
			break ;
		tmp = queue->heap[i];
		queue->heap[i] = queue->heap[parent];
		queue->heap[parent] = tmp;
		i = parent;
	}
}

void	heapify_up(t_queue *queue, int i)
{
	t_coder	*tmp;
	int		parent;

	while (i > 0)
	{
		parent = (i - 1) / 2;
		if (!priority_queue(queue->heap[i], queue->heap[parent]))
			break ;
		tmp = queue->heap[i];
		queue->heap[i] = queue->heap[parent];
		queue->heap[parent] = tmp;
		i = parent;
	}
}

void	heapify_down(t_queue *queue, int i)
{
	t_coder	*tmp;
	int		left;
	int		right;
	int		best;

	while (1)
	{
		left = 2 * i + 1;
		right = 2 * i + 2;
		best = i;
		if (left < queue->size
			&& priority_queue(queue->heap[left], queue->heap[best]))
			best = left;
		if (right < queue->size
			&& priority_queue(queue->heap[right], queue->heap[best]))
			best = right;
		if (best == i)
			break ;
		tmp = queue->heap[i];
		queue->heap[i] = queue->heap[best];
		queue->heap[best] = tmp;
		i = best;
	}
}

void	queue_remove(t_queue *queue, t_coder *coder)
{
	int	i;

	i = 0;
	while (i < queue->size && queue->heap[i] != coder)
		i++;
	if (i == queue->size)
		return ;
	queue->heap[i] = queue->heap[queue->size - 1];
	queue->size--;
	heapify_up(queue, i);
	heapify_down(queue, i);
}
