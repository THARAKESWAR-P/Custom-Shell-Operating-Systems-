
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main() {

    printf("Start of the Process\n");

    sleep(10);
    printf("parent pid:    %d\n", getpid());

    for (int i=0; i<5; i++){
    
        pid_t children_pid = fork();
        
        if (children_pid == 0) {
            printf("child %d pid:    %d\n", i, getpid());
            for (int j=0; j<10; j++) {
                
                pid_t grands_pid = fork();
                
                if (grands_pid == 0) {
                    printf("grandchild %d--%d pid:    %d\n", i, j, getpid());
                    while (1) {;}
                    //sleep(60);
                    exit(0);
                }
                wait(NULL);
            }
            
            while (1) {;}
            //sleep(10);
            exit(0);
        }
        wait(NULL);
    }

    sleep(10);

    return 0;
}
