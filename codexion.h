#ifndef CODEXION_H
#define CODEXION_H

#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#define COMPILING 0
#define DEBUGGING 1
#define REFACTORING 2

typedef struct s_simulation t_simulation;

typedef struct s_dongle
{
	int id;
	int available; // 1 or 0
	int cooldown;
	// pthread_mutex_t mutex;
} t_dongle;

typedef struct s_coder
{
	int id;
	t_dongle *left;
	t_dongle *right;
	pthread_t thread;
	t_simulation *sim;
	long last_compile_start;
	long deadline;
	int request_order;
} t_coder;

typedef struct s_queue
{
	t_coder **heap;
	int size;
	int capacity;
	pthread_cond_t cond;
	pthread_mutex_t mutex;
} t_queue;


typedef struct s_simulation
{
	int number_of_coders;
	int time_to_burnout;
	int time_to_compile;
	int time_to_debug;
	int time_to_refactor;
	int number_of_compiles_required;
	int dongle_cooldown;
	char *scheduler;

	t_coder *coders;
	t_dongle *dongles;
	t_queue queue;
	int next_request_order;
} t_simulation;

int parse_args(t_simulation *sim, char **av);
void free_simulation(t_simulation *sim);
int init_coders(t_simulation *sim);
int init_dongles(t_simulation *sim);

#endif