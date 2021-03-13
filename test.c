//
// Created by andre on 3/13/2021.
//

#include "types.h"
#include "user.h"
#include "pstat.h"
#include "param.h"

int
test(void)
{
    int i;
    for(i = 1; i < 4; i++) {
        int pid = fork();
        if(pid > 0) {
            settickets(i * 10);
            sleep(1000);
            break;
        }
    }

    exit();
}
