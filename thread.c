/*
 * cc thread.c -lpthread Except.o Assert.o Lifo.o Hash.o List.o
 */

#include <pthread.h>
#include "Except.h"

#define NUM_THREADS     10
#define NUM_LAUNCHERS   10

void *launch(void *);
void *thread(void *);

int main(void)
{
    pthread_t   launchers[NUM_LAUNCHERS];

    int i;

    try
    {
        for (i = 0; i < NUM_LAUNCHERS; i++)
        {
            pthread_create(&launchers[i], NULL, launch, (void *)0);
        }
    
        for (i = 0; i < NUM_LAUNCHERS; i++)
        {
            pthread_join(launchers[i], NULL);
        }
    }
    catch(Throwable, e)
    {
        e->printTryTrace(0);
    }
    finally;
}


void *launch(void *arg)
{
    pthread_t   threads[NUM_THREADS];
    int         i;

    for (i = 0; i < NUM_THREADS; i++)
    {
        pthread_create(&threads[i], NULL, thread, (void *)0);
    }

    for (i = 0; i < NUM_THREADS; i++)
    {
        pthread_join(threads[i], NULL);
    }   
    printf("launch: reporting that all %d threads have terminated\n", i);

    return 0;
}

void *thread(void *arg)
{
    try
    {
        *((int *)0) = 0;        /* most probably causes runtime exception */
    }
    catch (RuntimeException, e)
    {
        e->printTryTrace(0);
    }
    finally;
}
