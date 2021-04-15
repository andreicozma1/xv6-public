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
    int t[3] = {10, 20, 30};
    for(i = 1; i < 4; i++) {
        int pid = fork();
        if(pid > 0) {
            settickets(t[i-1]);
            while(1);
        }
    }

    exit();
}
