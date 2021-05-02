//
// Created by andre on 5/1/2021.
//

#include "types.h"
#include "stat.h"
#include "user.h"

struct args {
    int thread_num;
    int print_len;
    int* shared_var;
};

void
thread(void arg1, void arg2) {
    // get all need args from arg1 and arg2
    struct args args = ((struct args) arg1);
//  int thread_num = args->thread_num;
    int print_len = args->print_len;
    int *shared_var = args->shared_var;
    struct lock_t *lock = (struct lock_t*)arg2;

    // iterate print_len times and increment and print shared value inside
    // of lock
    for (int i = 0; i < print_len; i++) {
        lock_acquire(lock);
        (*shared_var)++;
//    printf(1, "thread: %d, shared_var = %d\n", thread_num, *shared_var);
        lock_release(lock);
    }
    free(args);
    exit();
}

void
threadtest()
{
    // set up variables
    int thread_num;
    int thread_count = 5;
    int print_len = 1000;
    int shared_var = (int ) malloc(sizeof(int));
    struct lock_t lock = (struct lock_t ) malloc(sizeof(struct lock_t));

    *shared_var = 0;
    lock_init(lock);

    // span thread_count many threads
    printf(1, "creating %d threads\n", thread_count);
    for (thread_num = 1; thread_num <= thread_count; thread_num++) {
        // create and args struct
        struct args args = (struct args) malloc(sizeof(struct args));
        args->thread_num = thread_num;
        args->print_len = print_len;
        args->shared_var = shared_var;

        // create the thread
        thread_create(thread, args, lock);
    }

    //join thread_count many threads
    for (thread_num = 1; thread_num <= thread_count; thread_num++) {
        thread_join();
    }
    printf(1, "all threads finished\n");
    printf(1, "shared variable: %d\n", *shared_var);

    free(lock);
}

int
main(void)
{
    threadtest();
    exit();
}