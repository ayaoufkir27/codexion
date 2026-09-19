/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   codexion.h                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ayoufkir <ayoufkir@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/19 12:38:29 by ayoufkir          #+#    #+#             */
/*   Updated: 2026/09/19 12:40:07 by ayoufkir         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CODEXION_H
# define CODEXION_H

# include <pthread.h>
# include <stdlib.h>
# include <unistd.h>
# include <stdio.h>
# include <string.h>
# include <sys/time.h>

typedef struct s_simulation	t_simulation;
typedef struct s_dongle
{
	int				id;
	int				available;
	long			free_at;
	pthread_mutex_t	mutex;
}	t_dongle;

typedef struct s_coder
{
	int				id;
	t_dongle		*left;
	t_dongle		*right;
	pthread_t		thread;
	t_simulation	*sim;
	long			deadline;
	int				request_order;
	int				done;
}	t_coder;

typedef struct s_queue
{
	t_coder			**heap;
	int				size;
	int				capacity;
	pthread_cond_t	cond;
	pthread_mutex_t	mutex;
}	t_queue;

typedef struct s_simulation
{
	int			number_of_coders;
	int			time_to_burnout;
	int			time_to_compile;
	int			time_to_debug;
	int			time_to_refactor;
	int			number_of_compiles_required;
	int			dongle_cooldown;
	char		*scheduler;

	t_coder		*coders;
	t_dongle	*dongles;
	t_queue		queue;
	int			next_request_order;
	long		start;
	pthread_t	monitor;
	int			stop;
}	t_simulation;

int		check_args(t_simulation *sim, char **av, int ac);
int		parse_args(t_simulation *sim, char **av, int ac);
void	free_simulation(t_simulation *sim);

int		init_coders(t_simulation *sim);
int		init_dongles(t_simulation *sim);
int		init_queue(t_simulation *sim);
int		create_coders(t_simulation *sim);
void	join_coders(t_simulation *sim);

long	elapsed_ms(t_simulation *sim);
long	get_time_ms(void);
void	request_dongles(t_coder *coder, long now);
void	release_dongles(t_coder *coder);
int		is_dongle_free(t_dongle *d, long now);
int		priority_queue(t_coder *a, t_coder *b);
int		acquire_dongles(t_coder *coder, t_simulation *sim);

void	queue_push(t_queue *queue, t_coder *coder);
void	heapify_queue(t_queue *queue, int i);
void	queue_remove(t_queue *queue, t_coder *coder);
void	*monitor_routine(void *arg);
void	wait_short(pthread_cond_t *cond, pthread_mutex_t *mutex);
void	*coder_routine(void *arg);

#endif