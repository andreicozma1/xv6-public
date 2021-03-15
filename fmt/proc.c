2400 #include "types.h"
2401 #include "defs.h"
2402 #include "param.h"
2403 #include "memlayout.h"
2404 #include "mmu.h"
2405 #include "x86.h"
2406 #include "proc.h"
2407 #include "spinlock.h"
2408 #include "pstat.h"
2409 #include "random.c"
2410 
2411 struct {
2412   struct spinlock lock;
2413   struct proc proc[NPROC];
2414   /* ANDREI: Added totaltickets counter used to determine cpu time allotted per process */
2415   int total_tickets;
2416 } ptable;
2417 
2418 static struct proc *initproc;
2419 
2420 int nextpid = 1;
2421 extern void forkret(void);
2422 extern void trapret(void);
2423 
2424 static void wakeup1(void *chan);
2425 
2426 void
2427 pinit(void)
2428 {
2429   initlock(&ptable.lock, "ptable");
2430 }
2431 
2432 // Must be called with interrupts disabled
2433 int
2434 cpuid() {
2435   return mycpu()-cpus;
2436 }
2437 
2438 
2439 
2440 
2441 
2442 
2443 
2444 
2445 
2446 
2447 
2448 
2449 
2450 // Must be called with interrupts disabled to avoid the caller being
2451 // rescheduled between reading lapicid and running through the loop.
2452 struct cpu*
2453 mycpu(void)
2454 {
2455   int apicid, i;
2456 
2457   if(readeflags()&FL_IF)
2458     panic("mycpu called with interrupts enabled\n");
2459 
2460   apicid = lapicid();
2461   // APIC IDs are not guaranteed to be contiguous. Maybe we should have
2462   // a reverse map, or reserve a register to store &cpus[i].
2463   for (i = 0; i < ncpu; ++i) {
2464     if (cpus[i].apicid == apicid)
2465       return &cpus[i];
2466   }
2467   panic("unknown apicid\n");
2468 }
2469 
2470 // Disable interrupts so that we are not rescheduled
2471 // while reading proc from the cpu structure
2472 struct proc*
2473 myproc(void) {
2474   struct cpu *c;
2475   struct proc *p;
2476   pushcli();
2477   c = mycpu();
2478   p = c->proc;
2479   popcli();
2480   return p;
2481 }
2482 
2483 
2484 
2485 
2486 
2487 
2488 
2489 
2490 
2491 
2492 
2493 
2494 
2495 
2496 
2497 
2498 
2499 
2500 // Look in the process table for an UNUSED proc.
2501 // If found, change state to EMBRYO and initialize
2502 // state required to run in the kernel.
2503 // Otherwise return 0.
2504 static struct proc*
2505 allocproc(void)
2506 {
2507   struct proc *p;
2508   char *sp;
2509 
2510   acquire(&ptable.lock);
2511 
2512   for(p = ptable.proc; p < &ptable.proc[NPROC]; p++)
2513     if(p->state == UNUSED)
2514       goto found;
2515 
2516   release(&ptable.lock);
2517   return 0;
2518 
2519 found:
2520   p->state = EMBRYO;
2521   p->pid = nextpid++;
2522 
2523   /* ANDREI: Added tickets field default value of 1 */
2524   p->tickets = 1;
2525   ptable.total_tickets += 1;
2526   p->ticks = 0;
2527 
2528   release(&ptable.lock);
2529 
2530   // Allocate kernel stack.
2531   if((p->kstack = kalloc()) == 0){
2532     p->state = UNUSED;
2533     return 0;
2534   }
2535   sp = p->kstack + KSTACKSIZE;
2536 
2537   // Leave room for trap frame.
2538   sp -= sizeof *p->tf;
2539   p->tf = (struct trapframe*)sp;
2540 
2541   // Set up new context to start executing at forkret,
2542   // which returns to trapret.
2543   sp -= 4;
2544   *(uint*)sp = (uint)trapret;
2545 
2546   sp -= sizeof *p->context;
2547   p->context = (struct context*)sp;
2548   memset(p->context, 0, sizeof *p->context);
2549   p->context->eip = (uint)forkret;
2550   return p;
2551 }
2552 
2553 
2554 // Set up first user process.
2555 void
2556 userinit(void)
2557 {
2558   struct proc *p;
2559   extern char _binary_initcode_start[], _binary_initcode_size[];
2560 
2561   p = allocproc();
2562 
2563   initproc = p;
2564   if((p->pgdir = setupkvm()) == 0)
2565     panic("userinit: out of memory?");
2566   inituvm(p->pgdir, _binary_initcode_start, (int)_binary_initcode_size);
2567   p->sz = PGSIZE;
2568   memset(p->tf, 0, sizeof(*p->tf));
2569   p->tf->cs = (SEG_UCODE << 3) | DPL_USER;
2570   p->tf->ds = (SEG_UDATA << 3) | DPL_USER;
2571   p->tf->es = p->tf->ds;
2572   p->tf->ss = p->tf->ds;
2573   p->tf->eflags = FL_IF;
2574   p->tf->esp = PGSIZE;
2575   p->tf->eip = 0;  // beginning of initcode.S
2576 
2577   safestrcpy(p->name, "initcode", sizeof(p->name));
2578   p->cwd = namei("/");
2579 
2580   // this assignment to p->state lets other cores
2581   // run this process. the acquire forces the above
2582   // writes to be visible, and the lock is also needed
2583   // because the assignment might not be atomic.
2584   acquire(&ptable.lock);
2585 
2586   p->state = RUNNABLE;
2587 
2588   release(&ptable.lock);
2589 }
2590 
2591 
2592 
2593 
2594 
2595 
2596 
2597 
2598 
2599 
2600 // Grow current process's memory by n bytes.
2601 // Return 0 on success, -1 on failure.
2602 int
2603 growproc(int n)
2604 {
2605   uint sz;
2606   struct proc *curproc = myproc();
2607 
2608   sz = curproc->sz;
2609   if(n > 0){
2610     if((sz = allocuvm(curproc->pgdir, sz, sz + n)) == 0)
2611       return -1;
2612   } else if(n < 0){
2613     if((sz = deallocuvm(curproc->pgdir, sz, sz + n)) == 0)
2614       return -1;
2615   }
2616   curproc->sz = sz;
2617   switchuvm(curproc);
2618   return 0;
2619 }
2620 
2621 // Create a new process copying p as the parent.
2622 // Sets up stack to return as if from system call.
2623 // Caller must set state of returned proc to RUNNABLE.
2624 int
2625 fork(void)
2626 {
2627   int i, pid;
2628   struct proc *np;
2629   struct proc *curproc = myproc();
2630 
2631   // Allocate process.
2632   if((np = allocproc()) == 0){
2633     return -1;
2634   }
2635 
2636   // Copy process state from proc.
2637   if((np->pgdir = copyuvm(curproc->pgdir, curproc->sz)) == 0){
2638     kfree(np->kstack);
2639     np->kstack = 0;
2640     np->state = UNUSED;
2641     return -1;
2642   }
2643   np->sz = curproc->sz;
2644   np->parent = curproc;
2645   *np->tf = *curproc->tf;
2646   /* ANDREI: Child tickets starts the same as parent tickets */
2647   np->tickets = curproc->tickets;
2648 
2649 
2650   // Clear %eax so that fork returns 0 in the child.
2651   np->tf->eax = 0;
2652 
2653   for(i = 0; i < NOFILE; i++)
2654     if(curproc->ofile[i])
2655       np->ofile[i] = filedup(curproc->ofile[i]);
2656   np->cwd = idup(curproc->cwd);
2657 
2658   safestrcpy(np->name, curproc->name, sizeof(curproc->name));
2659 
2660   pid = np->pid;
2661 
2662   acquire(&ptable.lock);
2663 
2664   np->state = RUNNABLE;
2665 
2666   release(&ptable.lock);
2667 
2668   return pid;
2669 }
2670 
2671 // Exit the current process.  Does not return.
2672 // An exited process remains in the zombie state
2673 // until its parent calls wait() to find out it exited.
2674 void
2675 exit(void)
2676 {
2677   struct proc *curproc = myproc();
2678   struct proc *p;
2679   int fd;
2680 
2681   if(curproc == initproc)
2682     panic("init exiting");
2683 
2684   // Close all open files.
2685   for(fd = 0; fd < NOFILE; fd++){
2686     if(curproc->ofile[fd]){
2687       fileclose(curproc->ofile[fd]);
2688       curproc->ofile[fd] = 0;
2689     }
2690   }
2691 
2692   begin_op();
2693   iput(curproc->cwd);
2694   end_op();
2695   curproc->cwd = 0;
2696 
2697   acquire(&ptable.lock);
2698 
2699 
2700   /* ANDREI: decrement total_tickets */
2701   ptable.total_tickets -= curproc->tickets;
2702   curproc->tickets = 0;
2703   curproc->ticks = 0;
2704 
2705     // Parent might be sleeping in wait().
2706   wakeup1(curproc->parent);
2707 
2708   // Pass abandoned children to init.
2709   for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
2710     if(p->parent == curproc){
2711       p->parent = initproc;
2712       if(p->state == ZOMBIE)
2713         wakeup1(initproc);
2714     }
2715   }
2716 
2717   // Jump into the scheduler, never to return.
2718   curproc->state = ZOMBIE;
2719   sched();
2720   panic("zombie exit");
2721 }
2722 
2723 // Wait for a child process to exit and return its pid.
2724 // Return -1 if this process has no children.
2725 int
2726 wait(void)
2727 {
2728   struct proc *p;
2729   int havekids, pid;
2730   struct proc *curproc = myproc();
2731 
2732   acquire(&ptable.lock);
2733   for(;;){
2734     // Scan through table looking for exited children.
2735     havekids = 0;
2736     for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
2737       if(p->parent != curproc)
2738         continue;
2739       havekids = 1;
2740       if(p->state == ZOMBIE){
2741         // Found one.
2742         pid = p->pid;
2743         kfree(p->kstack);
2744         p->kstack = 0;
2745         freevm(p->pgdir);
2746         p->pid = 0;
2747         p->parent = 0;
2748         p->name[0] = 0;
2749         p->killed = 0;
2750         p->state = UNUSED;
2751         release(&ptable.lock);
2752         return pid;
2753       }
2754     }
2755 
2756     // No point waiting if we don't have any children.
2757     if(!havekids || curproc->killed){
2758       release(&ptable.lock);
2759       return -1;
2760     }
2761 
2762     // Wait for children to exit.  (See wakeup1 call in proc_exit.)
2763     sleep(curproc, &ptable.lock);  //DOC: wait-sleep
2764   }
2765 }
2766 
2767 
2768 
2769 
2770 
2771 
2772 
2773 
2774 
2775 
2776 
2777 
2778 
2779 
2780 
2781 
2782 
2783 
2784 
2785 
2786 
2787 
2788 
2789 
2790 
2791 
2792 
2793 
2794 
2795 
2796 
2797 
2798 
2799 
2800 // Per-CPU process scheduler.
2801 // Each CPU calls scheduler() after setting itself up.
2802 // Scheduler never returns.  It loops, doing:
2803 //  - choose a process to run
2804 //  - swtch to start running that process
2805 //  - eventually that process transfers control
2806 //      via swtch back to the scheduler.
2807 void
2808 scheduler(void)
2809 {
2810   /* ANDREI: Declare variable for lottery winner */
2811   int counter, winner;
2812   struct proc *p;
2813   struct cpu *c = mycpu();
2814   c->proc = 0;
2815 
2816   for(;;){
2817     // Enable interrupts on this processor.
2818     sti();
2819 
2820     /* ANDREI:  pick random number for lottery */
2821     counter = 0;
2822     winner = randomrange(0, ptable.total_tickets);
2823 
2824 
2825 
2826     // Loop over process table looking for process to run.
2827     acquire(&ptable.lock);
2828     for(p = &ptable.proc[0]; p < &ptable.proc[NPROC]; p++){
2829       if(p->state != RUNNABLE)
2830         continue;
2831 
2832       /* ANDREI: Implement lottery scheduler algorithm */
2833       counter = counter + p->tickets;
2834 
2835       if(counter > winner) {
2836           // Switch to chosen process.  It is the process's job
2837           // to release ptable.lock and then reacquire it
2838           // before jumping back to us.
2839 
2840           c->proc = p;
2841           switchuvm(p);
2842           p->state = RUNNING;
2843 
2844           /* ANDREI: Increment ticks of process running */
2845           cprintf("%d, %d, %d, %d\n", , p->pid, p->tickets, p->ticks);
2846           p->ticks = p->ticks + 1;
2847 
2848 
2849 
2850           swtch(&(c->scheduler), p->context);
2851           switchkvm();
2852 
2853           // Process is done running for now.
2854           // It should have changed its p->state before coming back.
2855           c->proc = 0;
2856           break;
2857       }
2858     }
2859     release(&ptable.lock);
2860   }
2861 }
2862 
2863 // Enter scheduler.  Must hold only ptable.lock
2864 // and have changed proc->state. Saves and restores
2865 // intena because intena is a property of this
2866 // kernel thread, not this CPU. It should
2867 // be proc->intena and proc->ncli, but that would
2868 // break in the few places where a lock is held but
2869 // there's no process.
2870 void
2871 sched(void)
2872 {
2873   int intena;
2874   struct proc *p = myproc();
2875 
2876   if(!holding(&ptable.lock))
2877     panic("sched ptable.lock");
2878   if(mycpu()->ncli != 1)
2879     panic("sched locks");
2880   if(p->state == RUNNING)
2881     panic("sched running");
2882   if(readeflags()&FL_IF)
2883     panic("sched interruptible");
2884   intena = mycpu()->intena;
2885   swtch(&p->context, mycpu()->scheduler);
2886   mycpu()->intena = intena;
2887 }
2888 
2889 // Give up the CPU for one scheduling round.
2890 void
2891 yield(void)
2892 {
2893   acquire(&ptable.lock);  //DOC: yieldlock
2894   myproc()->state = RUNNABLE;
2895   sched();
2896   release(&ptable.lock);
2897 }
2898 
2899 
2900 // A fork child's very first scheduling by scheduler()
2901 // will swtch here.  "Return" to user space.
2902 void
2903 forkret(void)
2904 {
2905   static int first = 1;
2906   // Still holding ptable.lock from scheduler.
2907   release(&ptable.lock);
2908 
2909   if (first) {
2910     // Some initialization functions must be run in the context
2911     // of a regular process (e.g., they call sleep), and thus cannot
2912     // be run from main().
2913     first = 0;
2914     iinit(ROOTDEV);
2915     initlog(ROOTDEV);
2916   }
2917 
2918   // Return to "caller", actually trapret (see allocproc).
2919 }
2920 
2921 // Atomically release lock and sleep on chan.
2922 // Reacquires lock when awakened.
2923 void
2924 sleep(void *chan, struct spinlock *lk)
2925 {
2926   struct proc *p = myproc();
2927 
2928   if(p == 0)
2929     panic("sleep");
2930 
2931   if(lk == 0)
2932     panic("sleep without lk");
2933 
2934   // Must acquire ptable.lock in order to
2935   // change p->state and then call sched.
2936   // Once we hold ptable.lock, we can be
2937   // guaranteed that we won't miss any wakeup
2938   // (wakeup runs with ptable.lock locked),
2939   // so it's okay to release lk.
2940   if(lk != &ptable.lock){  //DOC: sleeplock0
2941     acquire(&ptable.lock);  //DOC: sleeplock1
2942     release(lk);
2943   }
2944   // Go to sleep.
2945   p->chan = chan;
2946   p->state = SLEEPING;
2947 
2948   sched();
2949 
2950   // Tidy up.
2951   p->chan = 0;
2952 
2953   // Reacquire original lock.
2954   if(lk != &ptable.lock){  //DOC: sleeplock2
2955     release(&ptable.lock);
2956     acquire(lk);
2957   }
2958 }
2959 
2960 
2961 
2962 
2963 
2964 
2965 
2966 
2967 
2968 
2969 
2970 
2971 
2972 
2973 
2974 
2975 
2976 
2977 
2978 
2979 
2980 
2981 
2982 
2983 
2984 
2985 
2986 
2987 
2988 
2989 
2990 
2991 
2992 
2993 
2994 
2995 
2996 
2997 
2998 
2999 
3000 // Wake up all processes sleeping on chan.
3001 // The ptable lock must be held.
3002 static void
3003 wakeup1(void *chan)
3004 {
3005   struct proc *p;
3006 
3007   for(p = ptable.proc; p < &ptable.proc[NPROC]; p++)
3008     if(p->state == SLEEPING && p->chan == chan)
3009       p->state = RUNNABLE;
3010 }
3011 
3012 // Wake up all processes sleeping on chan.
3013 void
3014 wakeup(void *chan)
3015 {
3016   acquire(&ptable.lock);
3017   wakeup1(chan);
3018   release(&ptable.lock);
3019 }
3020 
3021 // Kill the process with the given pid.
3022 // Process won't exit until it returns
3023 // to user space (see trap in trap.c).
3024 int
3025 kill(int pid)
3026 {
3027   struct proc *p;
3028 
3029   acquire(&ptable.lock);
3030   for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
3031     if(p->pid == pid){
3032       p->killed = 1;
3033       // Wake process from sleep if necessary.
3034       if(p->state == SLEEPING)
3035         p->state = RUNNABLE;
3036 
3037       /* ANDREI: Decrement total tickets when process is killed */
3038       ptable.total_tickets -= p->tickets;
3039       p->tickets = 0;
3040       p->ticks = 0;
3041 
3042       release(&ptable.lock);
3043       return 0;
3044     }
3045   }
3046   release(&ptable.lock);
3047   return -1;
3048 }
3049 
3050 
3051 // Print a process listing to console.  For debugging.
3052 // Runs when user types ^P on console.
3053 // No lock to avoid wedging a stuck machine further.
3054 void
3055 procdump(void)
3056 {
3057   static char *states[] = {
3058   [UNUSED]    "unused",
3059   [EMBRYO]    "embryo",
3060   [SLEEPING]  "sleep ",
3061   [RUNNABLE]  "runble",
3062   [RUNNING]   "run   ",
3063   [ZOMBIE]    "zombie"
3064   };
3065   int i;
3066   struct proc *p;
3067   char *state;
3068   uint pc[10];
3069 
3070   for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
3071     if(p->state == UNUSED)
3072       continue;
3073     if(p->state >= 0 && p->state < NELEM(states) && states[p->state])
3074       state = states[p->state];
3075     else
3076       state = "???";
3077     cprintf("%d %s %s", p->pid, state, p->name);
3078     if(p->state == SLEEPING){
3079       getcallerpcs((uint*)p->context->ebp+2, pc);
3080       for(i=0; i<10 && pc[i] != 0; i++)
3081         cprintf(" %p", pc[i]);
3082     }
3083     /* ANDREI: add tickets to procdump */
3084     cprintf(" Tickets:%d Ticks:%d", p->tickets, p->ticks);
3085     cprintf("\n");
3086   }
3087   cprintf("TOTAL TICKETS: %d\n", ptable.total_tickets);
3088 }
3089 
3090 
3091 
3092 
3093 
3094 
3095 
3096 
3097 
3098 
3099 
3100 /* ANDREI: Implement settickets function
3101  * The function takes the number of tickets to assign to a process.
3102  * The number of tickets must be greater than 0.
3103  * If number of tickets < 1, return -1, otherwise return 0
3104  * */
3105 int
3106 settickets(int number)
3107 {
3108     struct proc* p;
3109     if(number < 1)
3110         return -1;
3111 
3112     p = myproc();
3113     ptable.total_tickets = ptable.total_tickets + (number - p->tickets);
3114     p->tickets = number;
3115 
3116     return 0;
3117 }
3118 
3119 /* ANDREI: Implement getpinfo function
3120  * The function takes in a pstat structure and fills it in with the process info
3121  * If the pstat struct pointer is invalid return -1, otherwise return 0
3122  * */
3123 int
3124 getpinfo(struct pstat *stat)
3125 {
3126     int p;
3127     if(!stat)
3128         return -1;
3129 
3130     for(p = 0; p < NPROC; p++) {
3131         stat->inuse[p] = ptable.proc[p].state != UNUSED;
3132         stat->tickets[p] = ptable.proc[p].tickets;
3133         stat->pid[p] = ptable.proc[p].pid;
3134         stat->ticks[p] = ptable.proc[p].ticks;
3135     }
3136     return 0;
3137 }
3138 
3139 
3140 
3141 
3142 
3143 
3144 
3145 
3146 
3147 
3148 
3149 
