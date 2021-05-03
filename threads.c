//
// Created by Andrei Cozma on 5/1/2021.
//
#include "types.h"
#include "user.h"


void
thread_func(void *arg1, void *arg2) {
    // Arg 1 is shared variable and arg2 is lock
    lock_t *lock = arg2;
    for (int i = 0; i < 100; i++) {
        lock_acquire(lock);
        int val = *((int *) arg1);
        sleep(1);
        val += 1;
        *((int *) arg1) = val;
        lock_release(lock);
    }
    exit();
}

void
test()
{
    // Create a shared variable for the threads to be able to acquire lock
    int *shared = malloc(sizeof(int));
    *shared = 0;
    // Create a lock for our thread and call lock_init
    lock_t *lock = malloc(sizeof(lock_t));
    lock_init(lock);

    // Create 5 threads to do operations on a shared variable
    int t;
    int num_threads = 10;
    printf(1, "Spinning up %d threads\n", num_threads);
    for (t = 1; t <= 50; t++) {
        // create the thread and give it the arguments structure and the lock
        int pid = thread_create(thread_func, shared, lock);
        printf(1, "Successfully created thread with PID %d.\n",pid);

    }

    // Join on all the threads till all have finished
//    for (t = 1; t <= num_threads; t++) {
//        int pid = thread_join();
//        printf(1, "Successfully joined thread with PID %d\n", pid);

//    }
//    printf(1, "Successfully joined on all threads.\n");


    printf(1, "Final Shared Var Value: %d\n", *shared);

    // free the memory used for the lock
    free(lock);
}

int
main(void)
{
    test();
    exit();
}