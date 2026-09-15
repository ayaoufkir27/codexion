#include "codexion.h"

void free_simulation(t_simulation *sim)
{
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

long get_time_ms()
{
    struct timeval time;
    gettimeofday(&time, NULL);
    return (time.tv_sec * 1000L + time.tv_usec / 1000L);
}

long elapsed_ms(t_simulation *sim)
{
    return (get_time_ms() - sim->start);
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
        sim->coders[i].left = &sim->dongles[i];
        sim->coders[i].deadline = sim->time_to_burnout;
        sim->coders[i].done = 0;

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
        sim->dongles[i].available = 1;
        if (pthread_mutex_init(&sim->dongles[i].mutex, NULL) != 0)
            return 1;
        sim->dongles[i].free_at = 0;
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

    if (pthread_mutex_init(&sim->queue.mutex, NULL) != 0)
    {
        free(sim->queue.heap);
        return 1;
    }
    pthread_cond_init(&sim->queue.cond, NULL); // free
    return 0;
}

void request_dongles(t_coder *coder, long now)
{
    pthread_mutex_lock(&coder->left->mutex);
    coder->left->available = 0;
    pthread_mutex_unlock(&coder->left->mutex);
    printf("%ld %d has taken a dongle\n", now, coder->id);

    pthread_mutex_lock(&coder->right->mutex);
    coder->right->available = 0;
    pthread_mutex_unlock(&coder->right->mutex);
    printf("%ld %d has taken a dongle\n", now,coder->id);
}

void release_dongles(t_coder *coder)
{
    long now = elapsed_ms(coder->sim);
    // printf("TIME NOW: %ld\n", now);

    pthread_mutex_lock(&coder->left->mutex);
    coder->left->available = 1;
    coder->left->free_at = coder->sim->dongle_cooldown + now;
    pthread_mutex_unlock(&coder->left->mutex);
    // printf("C%d RELEASED left dongle %d\n", coder->id, coder->left->id);

    pthread_mutex_lock(&coder->right->mutex);
    coder->right->available = 1;
    coder->right->free_at = coder->sim->dongle_cooldown + now;
    pthread_mutex_unlock(&coder->right->mutex);
    // printf("C%d RELEASED right dongle %d\n", coder->id, coder->right->id);

}

int priority_queue(t_coder *a, t_coder *b)
{
    if (strcmp(a->sim->scheduler, "edf") == 0)
    {
        if (a->deadline != b->deadline)
            return (a->deadline < b->deadline);
        return (a->request_order < b->request_order);
    }
    return (a->request_order < b->request_order);
}

int is_dongle_free(t_dongle *d, long now)
{
    return (d->available && now >= d->free_at);
}

int aquire_dongles(t_coder *coder, t_simulation *sim)
{
    int i = 0;
    // int allowed = 1;
    t_coder *other;
    long now = elapsed_ms(sim);

    // pthread_mutex_lock(&coder->left->mutex);
    // if (!coder->left->available || now < coder->left->free_at)
    //     allowed = 0;
    // pthread_mutex_unlock(&coder->left->mutex);

    // pthread_mutex_lock(&coder->right->mutex);
    // if (!coder->right->available || now < coder->right->free_at)
    //     allowed = 0;
    // pthread_mutex_unlock(&coder->right->mutex);

    // if (!allowed)
    //     return 0;
    if (coder->left == coder->right)
        return 0;
    if (!is_dongle_free(coder->left, now) || !is_dongle_free(coder->right, now))
        return 0;
    while (i < sim->queue.size)
    {
        other = sim->queue.heap[i];
        if (coder != other
        && (coder->left == other->left || coder->right == other->right || coder->left == other->right || coder->right == other->left)
        && is_dongle_free(other->left, now)
        && is_dongle_free(other->right, now)
        && priority_queue(other, coder))
            return 0;
        i++;
    }
    return 1;
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

void queue_remove(t_queue *queue, t_coder *coder)
{
    int i = 0;
    t_coder *tmp;

    while (i < queue->size && queue->heap[i] != coder)
        i++;
    if (i == queue->size)
        return;

    queue->heap[i] = queue->heap[queue->size - 1];
    queue->size--;

    while (i > 0)
    {
        int parent = (i - 1) / 2;
        if (!priority_queue(queue->heap[i], queue->heap[parent]))
            break;
        tmp = queue->heap[i];
        queue->heap[i] = queue->heap[parent];
        queue->heap[parent] = tmp;

        i = parent;
    }

    while (1)
    {
        int left = 2 * i + 1;
        int right = 2 * i + 2;
        int best = i;

        if (left < queue->size && priority_queue(queue->heap[left], queue->heap[best]))
            best = left;
        if (right < queue->size && priority_queue(queue->heap[right], queue->heap[best]))
            best = right;
        if (best == i)
            break;
        
        tmp = queue->heap[i];
        queue->heap[i] = queue->heap[best];
        queue->heap[best] = tmp;

        i = best;
    }
}

void wait_short(pthread_cond_t *cond, pthread_mutex_t *mutex)
{
    struct timespec ts;
    struct timeval now;

    gettimeofday(&now, NULL);
    ts.tv_sec = now.tv_sec;
    ts.tv_nsec = (now.tv_usec + 5000) * 1000; // wake at least every 5ms
    if (ts.tv_nsec >= 1000000000)
    {
        ts.tv_sec += 1;
        ts.tv_nsec -= 1000000000;
    }
    pthread_cond_timedwait(cond, mutex, &ts);
}

void *monitor_routine(void *arg)
{
    t_simulation *sim = (t_simulation *)arg;
    long now;
    int i;
    int stop;

    while (1)
    {
        pthread_mutex_lock(&sim->queue.mutex);
        stop = sim->stop;
        if (!stop)
        {
            now = elapsed_ms(sim);
            i = 0;
            while(i < sim->number_of_coders)
            {
                if (now >= sim->coders[i].deadline && !sim->coders[i].done)
                {
                    printf("%ld %d burned out\n", now, sim->coders[i].id);
                    sim->stop = 1;
                    pthread_cond_broadcast(&sim->queue.cond);
                    break;
                }
                i++;
            }
        }
        pthread_mutex_unlock(&sim->queue.mutex);
        if (stop)
            break;
        usleep(1000);
    }
    return NULL;
}

void *coder_routine(void *arg)
{
    t_coder *coder = (t_coder *)arg;
    int i = 0;
    long now;
    int stop;

    while (i < coder->sim->number_of_compiles_required)
    {        
        pthread_mutex_lock(&coder->sim->queue.mutex);
        if (coder->sim->stop)
        {
            pthread_mutex_unlock(&coder->sim->queue.mutex);
            break;
        }
        coder->request_order = coder->sim->next_request_order;
        coder->sim->next_request_order++;
        // printf("[!] REQUEST ORDER %d\n", coder->request_order);
        // printf("[!] NEXT REQUEST ORDER %d\n", coder->sim->next_request_order);
        queue_push(&coder->sim->queue, coder);
        
        while(!aquire_dongles(coder, coder->sim) && !coder->sim->stop)
            wait_short(&coder->sim->queue.cond, &coder->sim->queue.mutex);

        if (coder->sim->stop)
        {
            queue_remove(&coder->sim->queue, coder);
            pthread_mutex_unlock(&coder->sim->queue.mutex);
            break;
        }

        printf("======= CODING ROUND %d=========\n", i + 1);
        now = elapsed_ms(coder->sim);
        request_dongles(coder, now);
        queue_remove(&coder->sim->queue, coder);
        coder->deadline = elapsed_ms(coder->sim) + coder->sim->time_to_burnout;
        pthread_mutex_unlock(&coder->sim->queue.mutex);
        
        now = elapsed_ms(coder->sim);
        printf("%ld %d is compiling\n", now, coder->id);
        // printf("=========== C%d DEADLINE: %ld\n", coder->id, coder->deadline);
        // printf("=========== C%d DEADLINE: %ld\n", coder->id, coder->deadline);
        usleep(coder->sim->time_to_compile * 1000);

        pthread_mutex_lock(&coder->sim->queue.mutex);
        release_dongles(coder);
        pthread_cond_broadcast(&coder->sim->queue.cond);
        pthread_mutex_unlock(&coder->sim->queue.mutex);

        pthread_mutex_lock(&coder->sim->queue.mutex);
        stop = coder->sim->stop;
        pthread_mutex_unlock(&coder->sim->queue.mutex);
        if (stop)
            break;
        
        now = elapsed_ms(coder->sim);
        printf("%ld %d is debugging\n", now, coder->id);
        usleep(coder->sim->time_to_debug * 1000);

        pthread_mutex_lock(&coder->sim->queue.mutex);
        stop = coder->sim->stop;
        pthread_mutex_unlock(&coder->sim->queue.mutex);
        if (coder->sim->stop)
            break;
        now = elapsed_ms(coder->sim);
        printf("%ld %d is refactoring\n", now, coder->id);
        usleep(coder->sim->time_to_refactor * 1000);

        i++;
        if (i == coder->sim->number_of_compiles_required)
            coder->done = 1;
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
    if (pthread_create(&sim->monitor, NULL, monitor_routine, sim) != 0)
        return 1;
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
    pthread_mutex_lock(&sim->queue.mutex);
    sim->stop = 1; // 
    pthread_mutex_unlock(&sim->queue.mutex);
    pthread_join(sim->monitor, NULL);
}

int main(int ac, char **av)
{
    t_simulation sim;
    sim.next_request_order = 1; // change initializing place
    sim.stop = 0;

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
    // test_queue(&sim);
    sim.start = get_time_ms();
    if (create_coders(&sim))
    {
        free_simulation(&sim);
        return 1;
    }
    join_coders(&sim);
    free_simulation(&sim);
    printf("program finished lol\n");
}
