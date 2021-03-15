3850 #include "types.h"
3851 #include "x86.h"
3852 #include "defs.h"
3853 #include "date.h"
3854 #include "param.h"
3855 #include "memlayout.h"
3856 #include "mmu.h"
3857 #include "proc.h"
3858 #include "pstat.h"
3859 
3860 int
3861 sys_fork(void)
3862 {
3863   return fork();
3864 }
3865 
3866 int
3867 sys_exit(void)
3868 {
3869   exit();
3870   return 0;  // not reached
3871 }
3872 
3873 int
3874 sys_wait(void)
3875 {
3876   return wait();
3877 }
3878 
3879 int
3880 sys_kill(void)
3881 {
3882   int pid;
3883 
3884   if(argint(0, &pid) < 0)
3885     return -1;
3886   return kill(pid);
3887 }
3888 
3889 int
3890 sys_getpid(void)
3891 {
3892   return myproc()->pid;
3893 }
3894 
3895 
3896 
3897 
3898 
3899 
3900 int
3901 sys_sbrk(void)
3902 {
3903   int addr;
3904   int n;
3905 
3906   if(argint(0, &n) < 0)
3907     return -1;
3908   addr = myproc()->sz;
3909   if(growproc(n) < 0)
3910     return -1;
3911   return addr;
3912 }
3913 
3914 int
3915 sys_sleep(void)
3916 {
3917   int n;
3918   uint ticks0;
3919 
3920   if(argint(0, &n) < 0)
3921     return -1;
3922   acquire(&tickslock);
3923   ticks0 = ticks;
3924   while(ticks - ticks0 < n){
3925     if(myproc()->killed){
3926       release(&tickslock);
3927       return -1;
3928     }
3929     sleep(&ticks, &tickslock);
3930   }
3931   release(&tickslock);
3932   return 0;
3933 }
3934 
3935 // return how many clock tick interrupts have occurred
3936 // since start.
3937 int
3938 sys_uptime(void)
3939 {
3940   uint xticks;
3941 
3942   acquire(&tickslock);
3943   xticks = ticks;
3944   release(&tickslock);
3945   return xticks;
3946 }
3947 
3948 
3949 
3950 /* ANDREI: for settickets system call
3951 * Uses the getpinfo function within proc.c to return the result
3952 * */
3953 int
3954 sys_settickets(void)
3955 {
3956     int result;
3957     int tickets;
3958     if(argint(0, &tickets) < 0)
3959         return -1;
3960 
3961     acquire(&tickslock);
3962     result = settickets(tickets);
3963     release(&tickslock);
3964 
3965     return result;
3966 }
3967 
3968 /* ANDREI: System call
3969  * Uses the getpinfo function within proc.c to return the result
3970  * */
3971 int
3972 sys_getpinfo(void)
3973 {
3974     char *p;
3975     if(argptr(0, &p, sizeof(struct pstat)) < 0)
3976         return -1;
3977 
3978     return getpinfo((struct pstat *) p);
3979 }
3980 
3981 
3982 
3983 
3984 
3985 
3986 
3987 
3988 
3989 
3990 
3991 
3992 
3993 
3994 
3995 
3996 
3997 
3998 
3999 
