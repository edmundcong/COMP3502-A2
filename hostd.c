/*** NO CHANGES TO THIS FILE ARE ALLOWED EXCEPT WHERE INDICATED ***/

/*** START OF SECTION MARKER ***/
/*** PLEASE UPDATE THIS SECTION. NO CODE INCLUDING INCLUDE STATEMENTS MAY BE ADDED. ***/
/*
    HOSTD - Hypothetical Operating System Testbed Dispatcher

    usage:

        hostd <TESTFILE>
        where <TESTFILE> is the name of a job list
*/

/************************************************************************************************************************

    ** Revision history **

    Current version: 1.1 bis
    Date: 7 April 2016

    1.1: Added instructions to allow additional functions in hostd.c and pcb.c
    1.0: Original version

    Contributors:
    1. COMP3520 teaching staff
       Centre for Distributed and High Performance Computing
       School of Information Technologies
       The University of Sydney
       NSW 2006
       Australia

    2. Dr Ian G Graham

    Copyright of this code and associated material is vested in the original contributors.

    This code is NOT in the Public Domain. Unauthorised posting of this code or derivatives thereof is not permitted.

    ** DO NOT REMOVE THIS NOTICE. Just update the revision history whenever you make changes to the code. **

 ***********************************************************************************************************************/
/*** END OF SECTION MARKER ***/

/* Include files */
#include "hostd.h"
#include "pcb.h"

/*** START OF SECTION MARKER ***/
/*** ADDITIONAL FUNCTION PROTOTYPES MAY BE ADDED HERE ***/
/*** END OF SECTION MARKER ***/

int main(int argc, char *argv[]) {
    /*** START OF SECTION MARKER ***/
    /*** You may add variable declarations in this section. ***/

    FILE *input_list_stream = NULL;
    // initialise dispatcher queues
    PcbPtr input_queue = NULL;
    PcbPtr hrrn_queue = NULL; // hrrn queue = priority 3 feedback queue
    PcbPtr fb_queue_p1 = NULL;
    PcbPtr fb_queue_p2 = NULL;
    PcbPtr current_process = NULL;
    PcbPtr process = NULL;
    int timer = 0;

/*** END OF SECTION MARKER ***/

/*** START OF SECTION MARKER ***/
/*** COMMENTS MAY BE ADDED, MODIFIED OR DELETED IN THIS SECTION ***/

//  1. Populate the FCFS queue

    if (argc == 0) {
        fprintf(stderr, "FATAL: Bad arguments array\n");
    } else if (argc != 2) {
        fprintf(stderr, "Usage: %s <TESTFILE>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    if (!(input_list_stream = fopen(argv[1], "r"))) {
        fprintf(stderr, "ERROR: Could not open \"%s\"\n", argv[1]);
        exit(EXIT_FAILURE);
    }

    while (1) {
        int val_check;
        process = createnullPcb();
        if ((val_check = fscanf(input_list_stream, "%d, %d, %d, %d",
                                &(process->arrival_time), &(process->priority),
                                &(process->scheduled_service_time), &(process->mbytes))) == 4) {
            process->remaining_cpu_time = process->scheduled_service_time;
            process->status = PCB_INITIALIZED;
            input_queue = enqPcb(input_queue, process);
        } else if (val_check >= 0) {
            free(process);
            fprintf(stderr, "ERROR: Invalid input file \"%s\"\n", argv[1]);
            exit(EXIT_FAILURE);
        } else if (ferror(input_list_stream)) {
            free(process);
            fprintf(stderr, "ERROR: Could not read input file \"%s\"\n", argv[1]);
            exit(EXIT_FAILURE);
        } else {
            free(process);
            break;
        }
    }
/*** END OF SECTION MARKER ***/

/*** START OF SECTION MARKER ***/
/*** CODE AND COMMENTS MAY BE ADDED, DELETED OR MODIFIED IN THIS SECTION ***/

/*** COMMENTS THAT BEGIN WITH // ARE FOR EXERCISE 4 ONLY ***/
/*** YOU NEED TO WRITE YOUR OWN COMMENTS ***/

    while (input_queue || hrrn_queue || fb_queue_p1 || fb_queue_p2 || current_process) {
        // unload pending process from the input queue?
        while (input_queue && input_queue->arrival_time <= timer) // unload any pending processes from input queue
        {
            // Dequeue process from input queue
            PcbPtr proc = deqPcb(&input_queue);
            int proc_priority = proc->priority;
            if (proc_priority == 1){
                // assign appropriate priority and quantu
                proc->time_quantum = 1;
                fb_queue_p1 = enqPcb(fb_queue_p1, proc);
            } else if (proc_priority == 2) {
                proc->time_quantum = 3;
                fb_queue_p2 = enqPcb(fb_queue_p2, proc);
            } else if (proc_priority == 3) {
                hrrn_queue = enqPcb_hd(hrrn_queue, proc);
            }
        }
        if (current_process) // if a process is currently running
        {
            current_process->remaining_cpu_time--;
            current_process->time_quantum--;
            if (current_process->remaining_cpu_time == 0) // if time is up
            {
                terminatePcb(current_process); // terminate
                free(current_process);
                current_process = NULL;
            }
            else if (current_process->priority == 1)
            {
                current_process->priority = 2;
                current_process->time_quantum = 3;
                if (fb_queue_p1 || fb_queue_p2 )
                {
                    suspendPcb(current_process);
                    fb_queue_p2 = enqPcb(fb_queue_p2, current_process);
                    current_process = NULL;
                }

            }
            else if (current_process->priority == 2)
            {
                if (current_process->time_quantum == 0)
                {
                    current_process->priority = 3;
                    if (fb_queue_p1 || fb_queue_p2 || hrrn_queue)
                    {
                        suspendPcb(current_process);
                        hrrn_queue = enqPcb(hrrn_queue, current_process);
                        current_process = NULL;
                    }
                } else if (fb_queue_p1)
                {
                    // higher level priority queue = fb1 queue
                    suspendPcb(current_process);
                    fb_queue_p2 = enqPcb_hd(fb_queue_p2, current_process);
                    current_process = NULL;
                }
            } else if ( fb_queue_p1 || fb_queue_p2 )
            {
                suspendPcb(current_process);
                hrrn_queue = (hrrn_queue, current_process);
                current_process = NULL;
            }
        }
        if (!(current_process) && (fb_queue_p1 || fb_queue_p2 || hrrn_queue)) // if no process currently running and HRRN queue not empty
        {
            if (fb_queue_p1 || fb_queue_p2)
            {
                if (fb_queue_p1) {
                    current_process = deqPcb(&fb_queue_p1);
                } else if (fb_queue_p2) {
                    current_process = deqPcb(&fb_queue_p2);
                }
            } else {
                current_process = deq_hrrn_Pcb(&hrrn_queue, timer);
            }
            startPcb(current_process);
        }
        sleep(1);
        timer++;
    }
    exit(EXIT_SUCCESS);
}

/*** END OF SECTION MARKER ***/

/*** START OF SECTION MARKER ***/
/*** ADDITIONAL FUNCTION IMPLEMENTATIONS MAY BE ADDED HERE ***/
/*** END OF SECTION MARKER ***/
/*** END OF CODE MARKER; DO NOT ADD MATERIAL BEYOND THIS POINT ***/