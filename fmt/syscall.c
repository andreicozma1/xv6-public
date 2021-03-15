3650 #include "types.h"
3651 #include "defs.h"
3652 #include "param.h"
3653 #include "memlayout.h"
3654 #include "mmu.h"
3655 #include "proc.h"
3656 #include "x86.h"
3657 #include "syscall.h"
3658 
3659 // User code makes a system call with INT T_SYSCALL.
3660 // System call number in %eax.
3661 // Arguments on the stack, from the user call to the C
3662 // library system call function. The saved user %esp points
3663 // to a saved program counter, and then the first argument.
3664 
3665 // Fetch the int at addr from the current process.
3666 int
3667 fetchint(uint addr, int *ip)
3668 {
3669   struct proc *curproc = myproc();
3670 
3671   if(addr >= curproc->sz || addr+4 > curproc->sz)
3672     return -1;
3673   *ip = *(int*)(addr);
3674   return 0;
3675 }
3676 
3677 // Fetch the nul-terminated string at addr from the current process.
3678 // Doesn't actually copy the string - just sets *pp to point at it.
3679 // Returns length of string, not including nul.
3680 int
3681 fetchstr(uint addr, char **pp)
3682 {
3683   char *s, *ep;
3684   struct proc *curproc = myproc();
3685 
3686   if(addr >= curproc->sz)
3687     return -1;
3688   *pp = (char*)addr;
3689   ep = (char*)curproc->sz;
3690   for(s = *pp; s < ep; s++){
3691     if(*s == 0)
3692       return s - *pp;
3693   }
3694   return -1;
3695 }
3696 
3697 
3698 
3699 
3700 // Fetch the nth 32-bit system call argument.
3701 int
3702 argint(int n, int *ip)
3703 {
3704   return fetchint((myproc()->tf->esp) + 4 + 4*n, ip);
3705 }
3706 
3707 // Fetch the nth word-sized system call argument as a pointer
3708 // to a block of memory of size bytes.  Check that the pointer
3709 // lies within the process address space.
3710 int
3711 argptr(int n, char **pp, int size)
3712 {
3713   int i;
3714   struct proc *curproc = myproc();
3715 
3716   if(argint(n, &i) < 0)
3717     return -1;
3718   if(size < 0 || (uint)i >= curproc->sz || (uint)i+size > curproc->sz)
3719     return -1;
3720   *pp = (char*)i;
3721   return 0;
3722 }
3723 
3724 // Fetch the nth word-sized system call argument as a string pointer.
3725 // Check that the pointer is valid and the string is nul-terminated.
3726 // (There is no shared writable memory, so the string can't change
3727 // between this check and being used by the kernel.)
3728 int
3729 argstr(int n, char **pp)
3730 {
3731   int addr;
3732   if(argint(n, &addr) < 0)
3733     return -1;
3734   return fetchstr(addr, pp);
3735 }
3736 
3737 
3738 
3739 
3740 
3741 
3742 
3743 
3744 
3745 
3746 
3747 
3748 
3749 
3750 extern int sys_chdir(void);
3751 extern int sys_close(void);
3752 extern int sys_dup(void);
3753 extern int sys_exec(void);
3754 extern int sys_exit(void);
3755 extern int sys_fork(void);
3756 extern int sys_fstat(void);
3757 extern int sys_getpid(void);
3758 extern int sys_kill(void);
3759 extern int sys_link(void);
3760 extern int sys_mkdir(void);
3761 extern int sys_mknod(void);
3762 extern int sys_open(void);
3763 extern int sys_pipe(void);
3764 extern int sys_read(void);
3765 extern int sys_sbrk(void);
3766 extern int sys_sleep(void);
3767 extern int sys_unlink(void);
3768 extern int sys_wait(void);
3769 extern int sys_write(void);
3770 extern int sys_uptime(void);
3771 /* ANDREI: Extern settickets & getpinfo systemcalls for lottery scheduler */
3772 extern int sys_settickets(void);
3773 extern int sys_getpinfo(void);
3774 
3775 static int (*syscalls[])(void) = {
3776 [SYS_fork]    sys_fork,
3777 [SYS_exit]    sys_exit,
3778 [SYS_wait]    sys_wait,
3779 [SYS_pipe]    sys_pipe,
3780 [SYS_read]    sys_read,
3781 [SYS_kill]    sys_kill,
3782 [SYS_exec]    sys_exec,
3783 [SYS_fstat]   sys_fstat,
3784 [SYS_chdir]   sys_chdir,
3785 [SYS_dup]     sys_dup,
3786 [SYS_getpid]  sys_getpid,
3787 [SYS_sbrk]    sys_sbrk,
3788 [SYS_sleep]   sys_sleep,
3789 [SYS_uptime]  sys_uptime,
3790 [SYS_open]    sys_open,
3791 [SYS_write]   sys_write,
3792 [SYS_mknod]   sys_mknod,
3793 [SYS_unlink]  sys_unlink,
3794 [SYS_link]    sys_link,
3795 [SYS_mkdir]   sys_mkdir,
3796 [SYS_close]   sys_close,
3797 /* ANDREI: Add settickets and getpinfo systemcalls to the array */
3798 [SYS_settickets]   sys_settickets,
3799 [SYS_getpinfo]   sys_getpinfo,
3800 };
3801 
3802 void
3803 syscall(void)
3804 {
3805   int num;
3806   struct proc *curproc = myproc();
3807 
3808   num = curproc->tf->eax;
3809   if(num > 0 && num < NELEM(syscalls) && syscalls[num]) {
3810     curproc->tf->eax = syscalls[num]();
3811   } else {
3812     cprintf("%d %s: unknown sys call %d\n",
3813             curproc->pid, curproc->name, num);
3814     curproc->tf->eax = -1;
3815   }
3816 }
3817 
3818 
3819 
3820 
3821 
3822 
3823 
3824 
3825 
3826 
3827 
3828 
3829 
3830 
3831 
3832 
3833 
3834 
3835 
3836 
3837 
3838 
3839 
3840 
3841 
3842 
3843 
3844 
3845 
3846 
3847 
3848 
3849 
