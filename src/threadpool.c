#include <stdlib.h>
#include <stdio.h>
#include "uthread.h"
#include "uthread_mutex_cond.h"

#include "threadpool.h"
#include "uthread_sem.h"

struct tpool {
    unsigned int threadNum;
//    unsigned int activeTasks;
    uthread_t *threads;

//    uthread_sem_t lock;
    uthread_mutex_t mutex;

//    uthread_sem_t hasElement;
    uthread_cond_t hasElement;


    struct task* head;
    struct task* tail;

    // 0 = cannot end, 1 = can end
    int canEnd;

};


struct task {
    void* id;
    void (*function)(tpool_t, void *);
    struct task* next;

};


struct task* create_task(void (*fun)(tpool_t, void *),
                         void *arg) {
    struct task* t = malloc(sizeof(struct task));
    t->id = arg;
    t->function = fun;
    t->next = NULL;
    return t;

}



void enqueue(tpool_t tp, struct task *toAdd) {
    if (toAdd == NULL) {
        return;
    }

    uthread_mutex_lock(tp->mutex);


    toAdd -> next = NULL;
    if(tp -> tail)
        tp -> tail -> next = toAdd;


    tp -> tail = toAdd;

    if(tp -> head == NULL)
        tp -> head = tp -> tail;


//    uthread_sem_signal(tp->hasElement);
    uthread_cond_broadcast(tp->hasElement);
    uthread_mutex_unlock(tp->mutex);


}


struct task *dequeue(tpool_t tp) {
    uthread_mutex_lock(tp->mutex);

    if (tp->head == NULL) {
        uthread_mutex_unlock(tp->mutex);
        return NULL;
    }


    struct task *toReturn = NULL;
    toReturn = tp->head;
    tp->head = tp->head->next;
    if(tp->head == NULL)
        tp -> tail = NULL;
    toReturn -> next = NULL;
    uthread_mutex_unlock(tp->mutex);

    return toReturn;
}

int isQueueEmpty(tpool_t pool) {
    return pool->head == NULL;
}



/* Function executed by each pool worker thread. This function is
 * responsible for running individual tasks. The function continues
 * running as long as either the pool is not yet joined, or there are
 * unstarted tasks to run. If there are no tasks to run, and the pool
 * has not yet been joined, the worker thread must be blocked.
 *
 * Parameter: param: The pool associated to the thread.
 * Returns: nothing.
 */
static void *worker_thread(void *param) {
    tpool_t pool = param;



    while (!pool->canEnd || !isQueueEmpty(pool)) {

//        uthread_mutex_lock(pool->mutex);
        while(isQueueEmpty(pool)) {
//            uthread_sem_wait(pool->canWork);
            uthread_mutex_lock(pool->mutex);
            uthread_cond_wait(pool->hasElement);
            uthread_mutex_unlock(pool->mutex);

        }
//        uthread_mutex_unlock(pool->mutex);


//        uthread_sem_wait(pool->hasElement);
//        uthread_sem_wait(pool->lock);
//        uthread_mutex_lock(pool->mutex);
        struct task *t = dequeue(pool);
//        uthread_sem_signal(pool->lock);
//        uthread_mutex_unlock(pool->mutex);
        if (t == NULL) {
            return NULL;
        }

//        pool->activeTasks++;
        t->function(pool, t->id);
    }



    return NULL;
}

/* Creates (allocates) and initializes a new thread pool. Also creates
 * `num_threads` worker threads associated to the pool, so that
 * `num_threads` tasks can run in parallel at any given time.
 *
 * Parameter: num_threads: Number of worker threads to be created.
 * Returns: a pointer to the new thread pool object.
 */
tpool_t tpool_create(unsigned int num_threads) {
    tpool_t tp = malloc(sizeof(struct tpool));
    tp->threadNum = num_threads;
//    tp->lock = uthread_sem_create(1);
    tp->mutex = uthread_mutex_create();
//    tp->hasElement = uthread_sem_create(0);
    tp->hasElement = uthread_cond_create(tp->mutex);
    tp->head = NULL;
    tp->tail = NULL;
    tp->threads = malloc(num_threads * sizeof (uthread_t));
    tp->canEnd = 0;
//    tp->activeTasks = 0;


    for (int i = 0; i < num_threads; ++i) {
        tp->threads[i] = uthread_create(worker_thread, tp);

    }

    return tp;


}

/* Queues a new task, to be executed by one of the worker threads
 * associated to the pool. The task is represented by function `fun`,
 * which receives the pool and a generic pointer as parameters. If any
 * of the worker threads is available, `fun` is started immediately by
 * one of the worker threads. If all of the worker threads are busy,
 * `fun` is scheduled to be executed when a worker thread becomes
 * available. Tasks are retrieved by individual worker threads in the
 * order in which they are scheduled, though due to the nature of
 * concurrency they may not start exactly in the same order. This
 * function returns immediately, and does not wait for `fun` to
 * complete.
 *
 * Parameters: pool: the pool that is expected to run the task.
 *             fun: the function that should be executed.
 *             arg: the argument to be passed to fun.
 */
void tpool_schedule_task(tpool_t pool, void (*fun)(tpool_t, void *),
                         void *arg) {
//    printf("SCHEDULE TASK\n");
    struct task* t = create_task(fun, arg);
//    printf("%d\n", __LINE__);

//    uthread_sem_wait(pool->lock);
//    uthread_mutex_lock(pool->mutex);
    enqueue(pool, t);
//    uthread_sem_signal(pool->lock);
//    uthread_mutex_unlock(pool->mutex);

}


void delete_tasks(struct task* tasks) {
    if (tasks == NULL) {
        return;
    }

    delete_tasks(tasks->next);
    free(tasks);
}


void delete_pool(tpool_t tpool) {
//    uthread_sem_destroy(tpool->hasElement);
//    uthread_sem_destroy(tpool->lock);
    uthread_mutex_destroy(tpool->mutex);
    uthread_cond_destroy(tpool->hasElement);

    delete_tasks(tpool->head);

    free(tpool->threads);
    free(tpool);

}



/* Blocks until the thread pool has no more scheduled tasks; then,
 * joins all worker threads, and frees the pool and all related
 * resources. Once this function returns, the pool cannot be used
 * anymore.
 *
 * Parameters: pool: the pool to be joined.
 */
void tpool_join(tpool_t pool) {
//      printf("%d\n", __LINE__);
    pool->canEnd = 1;

    uthread_mutex_lock(pool->mutex);
    uthread_cond_broadcast(pool->hasElement);
    uthread_mutex_unlock(pool->mutex);

    for (int i = 0; i < pool->threadNum; ++i) {
//        printf("%d\n", __LINE__);

        uthread_join(pool->threads[i], NULL);
//              printf("%d\n", __LINE__);

    }

    delete_pool(pool);
}

