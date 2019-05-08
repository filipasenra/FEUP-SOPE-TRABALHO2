struct thread_arg{
    sem_t *sem;
    pthread_mutex_t mutex;
};

void* box_office(void* arg);
