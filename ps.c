/* ANDREI: for settickets system call */

#include "types.h"
#include "user.h"
#include "pstat.h"
#include "param.h"

int
main(void)
{
    int i;
    struct pstat *stat = malloc(sizeof(struct pstat));
    int result = getpinfo(stat);
    if(result != 0)
        printf(2, "ps: cannot getpinfo\n");
    else
        for(i = 0; i < NPROC; i++)
            printf(0, "INUSE: %d  TICKETS: %d  PID: %d  TICKS: %d\n", stat->inuse[i], stat->tickets[i], stat->pid[i], stat->ticks[i]);

    exit();
}
