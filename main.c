#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "threadpool.h"
#include "list.h"

#define USAGE "usage: ./sort [thread_count] [input_count]\n"

struct timespec start, end;
double cpu_time;

struct {
    pthread_mutex_t mutex;
    int cut_thread_count;
} data_context;

static llist_t *tmp_list;
static llist_t *the_list = NULL;

static int thread_count = 0, data_count = 0, max_cut = 0;
static tpool_t *pool = NULL;

static double diff_in_second(struct timespec t1, struct timespec t2)
{
    struct timespec diff;
    if (t2.tv_nsec-t1.tv_nsec < 0) {
        diff.tv_sec  = t2.tv_sec - t1.tv_sec - 1;
        diff.tv_nsec = t2.tv_nsec - t1.tv_nsec + 1000000000;
    } else {
        diff.tv_sec  = t2.tv_sec - t1.tv_sec;
        diff.tv_nsec = t2.tv_nsec - t1.tv_nsec;
    }
    return (diff.tv_sec + diff.tv_nsec / 1000000000.0);
}

llist_t *merge_list(llist_t *a, llist_t *b)
{
    llist_t *_list = list_new();//the result list
    node_t *current = NULL;//act as the addition point of _list

    //neither of their length = 0
    while (a->size && b->size) {
        //small = MIN(a->head->data,b->head->data)
        llist_t *small = (llist_t *)
                         ((intptr_t) a * (a->head->data <= b->head->data) +
                          (intptr_t) b * (a->head->data > b->head->data));
        if (current) {//from second iteration
            current->next = small->head;
            current = current->next;
        } else {//first iteration
            _list->head = small->head;//put small list head first
            //and then left the _list pointer unchanged
            current = _list->head;//the current _list
        }
        small->head = small->head->next;
        --small->size;// small - 1
        ++_list->size;// list + 1
        current->next = NULL;
    }
    //remaining = last one
    llist_t *remaining = (llist_t *) ((intptr_t) a * (a->size > 0) +
                                      (intptr_t) b * (b->size > 0));
    if (current) current->next = remaining->head;//put the remaining
    _list->size += remaining->size;
    free(a);
    free(b);
    return _list;
}

llist_t *merge_sort(llist_t *list)//merge the same level list
{
    if (list->size < 2)//length = 1 or 0
        return list;
    int mid = list->size / 2;
    llist_t *left = list;
    llist_t *right = list_new();
    right->head = list_nth(list, mid);
    right->size = list->size - mid;
    list_nth(list, mid - 1)->next = NULL;
    left->size = mid;
    return merge_list(merge_sort(left), merge_sort(right));
}

void merge(void *data)//merge the cross level list
{
    llist_t *_list = (llist_t *) data;
    if (_list->size < (uint32_t) data_count) {
        pthread_mutex_lock(&(data_context.mutex));

        llist_t *_t = tmp_list;//list on the other side
        if (!_t) {
            tmp_list = _list;
            pthread_mutex_unlock(&(data_context.mutex));

        } else {
            tmp_list = NULL;
            pthread_mutex_unlock(&(data_context.mutex));

            task_t *_task = (task_t *) malloc(sizeof(task_t));
            _task->func = merge;
            _task->arg = merge_list(_list, _t);//old left list and new left list
            tqueue_push(pool->queue, _task);
        }
    } else {//terminal state: _list->size = data_count
        the_list = _list;
        //measure time
        clock_gettime(CLOCK_REALTIME, &end);
        cpu_time = diff_in_second(start,end);

        task_t *_task = (task_t *) malloc(sizeof(task_t));
        _task->func = NULL;
        tqueue_push(pool->queue, _task);
        //print result
        //list_print(_list);
    }
}

//dispatch the division task to queue
void cut_func(void *data)
{
    llist_t *llist = (llist_t *) data;

    pthread_mutex_lock(&(data_context.mutex));

    //cut thread once every iteration until achieving max_cut
    int cut_count = data_context.cut_thread_count;
    if (llist->size > 1 && cut_count < max_cut) {
        ++data_context.cut_thread_count;
        pthread_mutex_unlock(&(data_context.mutex));

        /* cut list to left list & right list*/
        //the remain one will go to right list
        int mid = llist->size / 2;
        llist_t *rlist = list_new();
        rlist->head = list_nth(llist, mid);
        rlist->size = llist->size - mid;
        list_nth(llist, mid-1)->next = NULL;
        llist->size = mid;

        /* create new task: left */
        task_t *_task = (task_t *) malloc(sizeof(task_t));
        _task->func = cut_func;
        _task->arg = llist;
        tqueue_push(pool->queue, _task);

        /* create new task: right */
        _task = (task_t *) malloc(sizeof(task_t));
        _task->func = cut_func;
        _task->arg = rlist;
        tqueue_push(pool->queue, _task);

    } else {//llist->size = 1 || cut_count = max_cut
        pthread_mutex_unlock(&(data_context.mutex));
        merge(merge_sort(llist));//no more cut thread, delegate the work
        //to merge_sort&merge
    }
}

//grab one task from task queue and excute
static void *task_run(void *data)
{
    (void) data;
    while (1) {
#if defined(MONITOR)
        pthread_mutex_lock(&(pool->queue->mutex));

        while( pool->queue->size == 0) {
            pthread_cond_wait(&(pool->queue->cond), &(pool->queue->mutex));
        }
        if (pool->queue->size == 0) break;
#endif
        task_t *_task = tqueue_pop(pool->queue);
#if defined(MONITOR)
        pthread_mutex_unlock(&(pool->queue->mutex));
#endif
        if (_task) {
            if (!_task->func) {//if function does not exist, push task back to queue
                tqueue_push(pool->queue, _task);
                break;
            } else {
                _task->func(_task->arg);//excute task
                free(_task);
            }
        }
    }
    pthread_exit(NULL);
}

int main(int argc, char const *argv[])
{
    if (argc < 3) {
        printf(USAGE);
        return -1;
    }
    thread_count = atoi(argv[1]);
    data_count = atoi(argv[2]);

    //max_cut = MIN(thread_count,data_count)-1
    max_cut = thread_count * (thread_count <= data_count) +
              data_count * (thread_count > data_count) - 1;

    /* Read data */
    the_list = list_new();

    //read data from input file
    FILE *fp = fopen("input","r");
    long int data;
    while((fscanf(fp,"%ld\n", &data)) != EOF) {
        list_add(the_list, data);
    }
    fclose(fp);
    /*
        printf("input unsorted data line-by-line\n");
        for (int i = 0; i < data_count; ++i) {
            long int data;
            scanf("%ld", &data);
            list_add(the_list, data);
        }
    */

    /* initialize tasks inside thread pool */
    pthread_mutex_init(&(data_context.mutex), NULL);

    data_context.cut_thread_count = 0;
    tmp_list = NULL;
    pool = (tpool_t *) malloc(sizeof(tpool_t));
    tpool_init(pool, thread_count, task_run);

    //measure time
    clock_gettime(CLOCK_REALTIME, &start);

    /* launch the first task */
    task_t *_task = (task_t *) malloc(sizeof(task_t));
    _task->func = cut_func;
    _task->arg = the_list;
    tqueue_push(pool->queue, _task);
    /* release thread pool */
    tpool_free(pool);

#if defined(BENCH)
    fp = fopen("output","a+");
    if(thread_count == 1) fprintf(fp, "%d,", data_count);
    fprintf(fp, "%lf", cpu_time);
    if(thread_count == 64) fprintf(fp, "\n");
    else fprintf(fp,",");
    fclose(fp);

    printf("BENCH: %d %d %lf\n", data_count, thread_count, cpu_time);

#elif defined(MONITOR)
    fp = fopen("monitor.txt","a+");
    fprintf(fp, "%lf", cpu_time);
    if(thread_count == 64) fprintf(fp,"\n");
    else fprintf(fp," ");
    fclose(fp);

    printf("MONITOR: %d %d %lf\n", data_count, thread_count, cpu_time);
#else
    fp = fopen("orig.txt","a+");
    fprintf(fp, "%lf", cpu_time);
    if(thread_count == 64) fprintf(fp,"\n");
    else fprintf(fp," ");
    fclose(fp);

    printf("ORIGINAL: %d %d %lf\n", data_count, thread_count, cpu_time);
#endif

    return 0;
}
