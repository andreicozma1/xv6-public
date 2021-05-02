//
// Created by Andrei Cozma on 5/1/2021.
//
#include "types.h"
#include "user.h"

struct args {
    int thread_number;
    int* common_variable;
};

void
thread_func(void *arg1, void *arg2) {
    // Retrieve the current therad number and common variable from arg1
    struct args *args = ((struct args*) arg1);
    // Arg2 is given to be the common lock
    lock_t *lock = arg2;

    // The common variable between the threads
    int *shared = args->common_variable;

    // Perform operation 5000 times with a lock, printing the result.
    for (int i = 0; i < 5000; i++) {
        lock_acquire(lock);
        (*shared)++;
        printf(1, "Thread #%d modified shared variable = %d\n", t, *shared);
        lock_release(lock);
    }
    // Free arguments and make thread exit
    free(args);
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
    int num_threads = 5;
    printf(1, "Spinning up %d threads\n", num_threads);
    for (t = 1; t <= 5; t++) {
        // Give the shared variable and thread number via arg1
        struct args *args = malloc(sizeof(struct args));
        args->thread_number = t;
        args->common_variable = shared;
        // create the thread and give it the arguments structure and the lock
        thread_create(thread_func, args, lock);
    }

    // Join on all the threads till all have finished
    for (t = 1; t <= num_threads; t++)
        thread_join();

    printf(1, "Successfully joined on all threads.\n");
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