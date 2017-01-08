#include "threadpool.h"

int task_free(task_t *the_task)
{
    free(the_task->arg);
    free(the_task);
    return 0;
}

int tqueue_init(tqueue_t *the_queue)
{
//    if((the_queue = (tqueue_t *)malloc(sizeof(tqueue_t)) == NULL)) return -1;
    the_queue->head = NULL;
    the_queue->tail = NULL;
    pthread_mutex_init(&(the_queue->mutex), NULL);
    pthread_cond_init(&(the_queue->cond), NULL);
    the_queue->size = 0;
    return 0;
}

task_t *tqueue_pop(tqueue_t *the_queue)
{
    task_t *ret;
    pthread_mutex_lock(&(the_queue->mutex));
    ret = the_queue->head;
    if (ret) {
        the_queue->head = ret->next;//if it is NULL, then let it be
        if (the_queue->head) {
            the_queue->head->last = NULL;
        } else {
            the_queue->tail = NULL;
        }
        the_queue->size--;
    }
    pthread_mutex_unlock(&(the_queue->mutex));
    return ret;
}

uint32_t tqueue_size(tqueue_t *the_queue)
{
    uint32_t ret;
    pthread_mutex_lock(&(the_queue->mutex));
    ret = the_queue->size;
    pthread_mutex_unlock(&(the_queue->mutex));
    return ret;
}

int tqueue_push(tqueue_t *the_queue, task_t *task)
{
    if(task == NULL) return -1;
    pthread_mutex_lock(&(the_queue->mutex));
    task->next = NULL;
    task->last = the_queue->tail;
    if (the_queue->tail)
        the_queue->tail->next = task;
    the_queue->tail = task; //only one task in queue
    if (the_queue->size++ == 0)
        the_queue->head = task;//only one task in queue
    pthread_mutex_unlock(&(the_queue->mutex));
    return 0;
}

int tqueue_free(tqueue_t *the_queue)
{
    if(the_queue == NULL) return -1;
    task_t *cur = the_queue->head;
    while (cur) {
        the_queue->head = the_queue->head->next;
        free(cur);
        cur = the_queue->head;
    }
    pthread_mutex_destroy(&(the_queue->mutex));
    pthread_cond_destroy(&(the_queue->cond));
    return 0;
}

int tpool_init(tpool_t *the_pool, uint32_t tcount, void *(*func)(void *))
{
    the_pool->threads = (pthread_t *) malloc(sizeof(pthread_t) * tcount);
    the_pool->count = tcount;
    the_pool->queue = (tqueue_t *) malloc(sizeof(tqueue_t));
    tqueue_init(the_pool->queue);
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    for (uint32_t i = 0; i < tcount; ++i)
        pthread_create(&(the_pool->threads[i]), &attr, func, NULL);
    pthread_attr_destroy(&attr);
    return 0;
}

int tpool_free(tpool_t *the_pool)
{
    if(the_pool == NULL) return -1;
    for (uint32_t i = 0; i < the_pool->count; ++i)
        if(pthread_join(the_pool->threads[i], NULL) != 0) return -1;
    free(the_pool->threads);
    tqueue_free(the_pool->queue);
    return 0;
}
