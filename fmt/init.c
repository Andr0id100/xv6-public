8750 // init: The initial user-level program
8751 
8752 #include "types.h"
8753 #include "stat.h"
8754 #include "user.h"
8755 #include "fcntl.h"
8756 
8757 char *argv[] = { "sh", 0 };
8758 
8759 int
8760 main(void)
8761 {
8762   int pid, wpid;
8763 
8764   if(open("console", O_RDWR) < 0){
8765     mknod("console", 1, 1);
8766     open("console", O_RDWR);
8767   }
8768   dup(0);  // stdout
8769   dup(0);  // stderr
8770 
8771   for(;;){
8772     printf(1, "init: starting sh\n");
8773     pid = fork();
8774     if(pid < 0){
8775       printf(1, "init: fork failed\n");
8776       exit();
8777     }
8778     if(pid == 0){
8779       exec("sh", argv);
8780       printf(1, "init: exec sh failed\n");
8781       exit();
8782     }
8783     while((wpid=wait()) >= 0 && wpid != pid)
8784       printf(1, "zombie!\n");
8785   }
8786 }
8787 
8788 
8789 
8790 
8791 
8792 
8793 
8794 
8795 
8796 
8797 
8798 
8799 
