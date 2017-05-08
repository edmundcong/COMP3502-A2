#include <stdio.h>
// provide access to POSIX
#include <unistd.h>
#include <signal.h>
#include "cmake-build-debug/pcb_1.c"


// to avoid implicit declaration warnings
int process_creation();

//int main() {
//    process_creation();
//    PcbPtr p = createnullPcb();
//    startPcb(p);
//    return 0;
//}

/* Write a short test program to create a process using fork and then use the child process to execute ./process
 * Then, use the parent process to use the kill function to terminat ethe child process after sleeping a few seconds */

int process_creation() {
    pid_t pid = fork(); // fork the program creating a child and parent
    if (pid == 0) // get child to exec process
    {
        char *args[2];
        args[0] = "./process"; // program must be in the same folder as main.c
        args[1] = NULL;
        execvp(args[0], args); // execute program
    }
    sleep(5); // sleep both processes for 5 seconds
    if (pid > 0) // parent process
    {
        // kill child process after 5 seconds with SIGINT
        if (kill(0, SIGINT)) fprintf(stderr, "terminate of %d failed", 0);
    }
    return 0;
}
