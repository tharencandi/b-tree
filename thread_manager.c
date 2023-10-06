

#include <pthread.h>

struct thread_pool {
    
    int total_threads;
    int free_threads;
    int locked_threads;
    pthread_t * free_threads;
    pthread_t * locked_threads;

};


void* use_thread(struct thread_pool * pool) {

    


}