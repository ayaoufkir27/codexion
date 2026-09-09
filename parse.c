#include "codexion.h"

void free_simulation(t_simulation *sim)
{
    // int i = 0;

    // while (i < sim->number_of_coders)
    // {
    //     pthread_mutex_destroy(&sim->dongles[i].mutex);
    //     i++;
    // }
    free(sim->coders);
    free(sim->dongles);
}

int parse_args(t_simulation *sim, char **av)
{
    sim->number_of_coders = atoi(av[1]);
    sim->time_to_burnout = atoi(av[2]);
    sim->time_to_compile = atoi(av[3]);
    sim->time_to_debug = atoi(av[4]);
    sim->time_to_refactor = atoi(av[5]);
    sim->number_of_compiles_required = atoi(av[6]);
    sim->dongle_cooldown = atoi(av[7]);
    sim->scheduler = av[8];

    if (strcmp(sim->scheduler, "fifo") != 0 && strcmp(sim->scheduler, "edf") != 0)
        return 1;

    return 0;
}

int init_coders(t_simulation *sim)
{
    int num = sim->number_of_coders;
    sim->coders = malloc(sizeof(t_coder) * num);
    if (!sim->coders)
        return 1;
    int i = 0;

    while (i < num)
    {
        sim->coders[i].id = i + 1;
        sim->coders[i].sim = sim;
        // sim->coders[i].state = 0;
        sim->coders[i].left = &sim->dongles[i];

        if (i == sim->number_of_coders - 1)
            sim->coders[i].right = &sim->dongles[0];
        else
            sim->coders[i].right = &sim->dongles[i + 1];
        // printf("Coder id: %d, ", sim->coders[i].id);
        // printf("left dongle: %d, ", sim->coders[i].left->id);
        // printf("right dongle: %d\n", sim->coders[i].right->id);
        i++;
    }
    return 0;
}

int init_dongles(t_simulation *sim)
{
    int num = sim->number_of_coders;
    int i = 0;
    sim->dongles = malloc(sizeof(t_dongle) * num);
    if (!sim->dongles)
        return 1;

    while (i < num)
    {
        sim->dongles[i].id = i + 1;
        if (pthread_mutex_init(&sim->dongles[i].mutex, NULL) != 0)
            return 1;
        i++;
    }
    return 0;
}

int init_queue(t_simulation *sim)
{
    sim->queue.capacity = sim->number_of_coders;
    sim->queue.size = 0;

    sim->queue.heap = malloc(sizeof(t_coder *) * sim->queue.capacity);
    if (!sim->queue.heap)
        return 1;

    return 0;
}

void request_dongles(t_coder *coder)
{
    pthread_mutex_lock(&coder->left->mutex);
    printf("Coder %d took left dongle id: %d\n", coder->id, coder->left->id);

    pthread_mutex_lock(&coder->right->mutex);
    printf("Coder %d took right dongle id: %d\n", coder->id, coder->right->id);
}
void release_dongles(t_coder *coder)
{
    pthread_mutex_unlock(&coder->left->mutex);
    printf("Coder %d released left dongle %d\n", coder->id, coder->left->id);

    pthread_mutex_unlock(&coder->right->mutex);
    printf("Coder %d released right dongle %d\n", coder->id, coder->right->id);
}

int priority_queue(t_coder *a, t_coder *b)
{
    return (a->request_order < b->request_order);
}

void queue_push(t_queue *queue, t_coder *coder)
{
    int parent;
    t_coder *tmp;
    int i = queue->size;
    queue->heap[i] = coder;
    queue->size++;

    while (i > 0)
    {
        parent = (i - 1) / 2;
        if (!priority_queue(queue->heap[i], queue->heap[parent]))
            break;

        tmp = queue->heap[i];
        queue->heap[i] = queue->heap[parent];
        queue->heap[parent] = tmp;

        i = parent;
    }
}

t_coder *queue_pop(t_queue *queue)
{
    t_coder *top = queue->heap[0];
    queue->heap[0] = queue->heap[queue->size - 1];
    queue->size--;

    int i = 0;
    t_coder *tmp;
    while (i < queue->size)
    {
        queue->heap[i] = queue->heap[i + 1];
        i++;
    }
    return top;
}

void test_queue(t_simulation *sim)
{
    sim->coders[0].request_order = 2;
    sim->coders[1].request_order = 0;
    sim->coders[2].request_order = 3;
    sim->coders[3].request_order = 1;

    queue_push(&sim->queue, &sim->coders[0]);
    queue_push(&sim->queue, &sim->coders[1]);
    queue_push(&sim->queue, &sim->coders[2]);
    queue_push(&sim->queue, &sim->coders[3]);

    queue_pop(&sim->queue);
    int i = 0;
    while (i < sim->queue.size)
    {
        printf("heap[%d] = Coder %d (order %d)\n",
            i,
            sim->queue.heap[i]->id,
            sim->queue.heap[i]->request_order);
        i++;
    }
}

void *coder_routine(void *arg)
{
    t_coder *coder = (t_coder *)arg;
    // printf("Coder %d is running\n", coder->id);
    int i = 0;
    while (i < coder->sim->number_of_compiles_required)
    {
        request_dongles(coder);

        printf("Coder %d is compiling\n", coder->id);
        usleep(coder->sim->time_to_compile * 1000);

        release_dongles(coder);

        printf("Coder %d is debugging\n", coder->id);
        usleep(coder->sim->time_to_debug * 1000);

        printf("Coder %d is refactoring\n", coder->id);
        usleep(coder->sim->time_to_refactor * 1000);

        i++;
    }
    return (NULL);
}

int create_coders(t_simulation *sim)
{
    int i = 0;
    while (i < sim->number_of_coders)
    {
        if (pthread_create(&sim->coders[i].thread, NULL, coder_routine, &sim->coders[i]) != 0)
            return 1;
        i++;
    }
    return 0;
}

void join_coders(t_simulation *sim)
{
    int i = 0;

    while (i < sim->number_of_coders)
    {
        pthread_join(sim->coders[i].thread, NULL);
        i++;
    }
}

int main(int ac, char **av)
{
    t_simulation sim;

    if (ac != 9)
        return 1;
    if (parse_args(&sim, av))
        return 1;
    if (init_dongles(&sim))
        return 1;
    if (init_coders(&sim))
    {
        free_simulation(&sim);
        return 1;
    }
    if (init_queue(&sim))
    {
        free_simulation(&sim);
        return 1;
    }
    test_queue(&sim);
    // if (create_coders(&sim))
    // {
    //     free_simulation(&sim);
    //     return 1;
    // }
    // join_coders(&sim);
    free_simulation(&sim);
    printf("program finished lol\n");
}
