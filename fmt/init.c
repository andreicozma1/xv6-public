8650 // init: The initial user-level program
8651 
8652 #include "types.h"
8653 #include "stat.h"
8654 #include "user.h"
8655 #include "fcntl.h"
8656 
8657 char *argv[] = { "sh", 0 };
8658 
8659 int
8660 main(void)
8661 {
8662   int pid, wpid;
8663 
8664   if(open("console", O_RDWR) < 0){
8665     mknod("console", 1, 1);
8666     open("console", O_RDWR);
8667   }
8668   dup(0);  // stdout
8669   dup(0);  // stderr
8670 
8671   for(;;){
8672     printf(1, "init: starting sh\n");
8673     pid = fork();
8674     if(pid < 0){
8675       printf(1, "init: fork failed\n");
8676       exit();
8677     }
8678     if(pid == 0){
8679       exec("sh", argv);
8680       printf(1, "init: exec sh failed\n");
8681       exit();
8682     }
8683     while((wpid=wait()) >= 0 && wpid != pid)
8684       printf(1, "zombie!\n");
8685   }
8686 }
8687 
8688 
8689 
8690 
8691 
8692 
8693 
8694 
8695 
8696 
8697 
8698 
8699 
