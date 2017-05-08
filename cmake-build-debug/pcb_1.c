/*** NO CHANGES TO THIS FILE ARE ALLOWED EXCEPT WHERE INDICATED ***/
/* PCB management functions for HOSTD */

/* Include Files */
#include "pcb.h"

/*** CODE COMMENTS MAY BE ADDED BEYOND THIS POINT ***/

/*** START OF SECTION MARKER ***/
/*** ADDITIONAL FUNCTION PROTOTYPES MAY BE ADDED HERE ***/
/*** END OF SECTION MARKER ***/

/*******************************************************
 * PcbPtr startPcb(PcbPtr process) - start (or restart)
 *    a process
 * returns:
 *    PcbPtr of process
 *    NULL if start (restart) failed
 ******************************************************/
PcbPtr startPcb (PcbPtr p)
{
    if (p->pid == 0)
    {
        switch (p->pid = fork())
        {
            case -1:
                fprintf(stderr, "FATAL: Could not fork process!\n");
                exit(EXIT_FAILURE);
            case 0:
                p->pid = getpid();
                p->status = PCB_RUNNING;
                printPcbHdr();
                printPcb(p);
                fflush(stdout);
                execv(p->args[0], p->args);
                fprintf(stderr, "Could not run process!\n");
                exit(EXIT_FAILURE);
        }
    }
    else
    {
        kill(p->pid, SIGCONT);
    }
    p->status = PCB_RUNNING;
    return p;
}

/*******************************************************
 * PcbPtr suspendPcb(PcbPtr process) - suspend
 *    a process
 * returns:
 *    PcbPtr of process
 *    NULL if suspension failed
 ******************************************************/
PcbPtr suspendPcb(PcbPtr p)
{
    if (!p)
    {
        fprintf(stderr, "ERROR: Can not suspend a NULL process\n");
        return NULL;
    }
    else
    {
        return NULL; /*** REPLACE THIS LINE WITH YOUR CODE ***/
    }
}

/*******************************************************
 * PcbPtr terminatePcb(PcbPtr process) - terminate
 *    a process
 * returns:
 *    PcbPtr of process
 *    NULL if termination failed
 ******************************************************/
PcbPtr terminatePcb(PcbPtr p)
{
    if (!p)
    {
        fprintf(stderr, "ERROR: Can not terminate a NULL process\n");
        return NULL;
    }
    else
    {
        // kill return value
        int kill_rv = kill(p->pid, SIGINT);
        if (!kill_rv) // returning 0 means that kill worked
            return p;
        else return NULL;
    }
}

/*******************************************************
 * PcbPtr printPcb(PcbPtr process)
 *  - print process attributes on stdout
 *  returns:
 *    PcbPtr of process
 ******************************************************/
PcbPtr printPcb(PcbPtr p)
{
    printf("%7d%7d%7d%7d  ",
           (int) p->pid, p->arrival_time, p->priority,
           p->remaining_cpu_time);
//        p->memoryblock->offset, p->mbytes,
    switch (p->status) {
        case PCB_UNINITIALIZED:
            printf("UNINITIALIZED");
            break;
        case PCB_INITIALIZED:
            printf("INITIALIZED");
            break;
        case PCB_READY:
            printf("READY");
            break;
        case PCB_RUNNING:
            printf("RUNNING");
            break;
        case PCB_SUSPENDED:
            printf("SUSPENDED");
            break;
        case PCB_TERMINATED:
            printf("PCB_TERMINATED");
            break;
        default:
            printf("UNKNOWN");
    }
    printf("\n");

    return p;
}

/*******************************************************
 * void printPcbHdr() - print header for printPcb
 *  returns:
 *    void
 ******************************************************/
void printPcbHdr()
{
    printf("    pid arrive  prior    cpu  status\n");

}

/*******************************************************
 * PcbPtr createnullPcb() - create inactive Pcb.
 *
 * returns:
 *    PcbPtr of newly initialised Pcb
 *    NULL if malloc failed
 ******************************************************/
PcbPtr createnullPcb()
{
    PcbPtr new_process_Ptr;
    if (!(new_process_Ptr = (PcbPtr)malloc(sizeof(Pcb))))
    {
        fprintf(stderr, "ERROR: Could not create new process control block\n");
        return NULL;
    }
    new_process_Ptr->pid = 0;
    new_process_Ptr->args[0] = "./process";
    new_process_Ptr->args[1] = NULL;
    new_process_Ptr->arrival_time = 0;
    new_process_Ptr->priority = 0;
    new_process_Ptr->remaining_cpu_time = 0;
    new_process_Ptr->mbytes = 0;
//    new_process_Ptr->mem_block = NULL;
    new_process_Ptr->status = PCB_UNINITIALIZED;
    new_process_Ptr->next = NULL;

    return new_process_Ptr;
}

/*******************************************************
 * PcbPtr enqPcb (PcbPtr headofQ, PcbPtr process)
 *    - queue process (or join queues) at end of queue
 *
 * returns head of queue
 ******************************************************/
PcbPtr enqPcb(PcbPtr q, PcbPtr p)
{
    if (q == NULL) return p;
    PcbPtr head = q;
    while (q->next != NULL)
    {
        q = q->next;
    }
    q->next = p;
    return head;
}

/*******************************************************
 * PcbPtr deqPcb (PcbPtr * headofQ);
 *    - dequeue process - take Pcb from head of queue.
 *
 * returns:
 *    PcbPtr if dequeued,
 *    NULL if queue was empty
 *    & sets new head of Q pointer in adrs at 1st arg
 *******************************************************/
PcbPtr deqPcb(PcbPtr * hPtr) // never free when de/en queueing
{
    // logical errors and queue is already empty
    if (hPtr == NULL || * hPtr == NULL) return NULL;
    PcbPtr temp_head = * hPtr; // save the old head to the head of our LL
    *hPtr = (*hPtr)->next; // update the head pointer to the next node in the LL
    temp_head->next = NULL; // sever old head
    return temp_head; // returning old head
}

PcbPtr deq_hrrn_Pcb(PcbPtr * queue_head_ptr, int timer) // pointer to pointer to affect data outside of function scope
{
    // logical errors           empty queue AND logical errors
    if (!(queue_head_ptr) || !(*queue_head_ptr )) return NULL;
    // keep track of ALL nodes
    PcbPtr temp_prev_node;
    PcbPtr temp_node = * queue_head_ptr;
    // keep track of max nodes
    PcbPtr dq_node = * queue_head_ptr;
    PcbPtr dq_prev_node = * queue_head_ptr;

    if (!temp_node->next) // if we only have 1 element in queue
    {
        return temp_node;
    }

    float response_ratio = 0; // RR will be >= 1 for jobs that have arrived
    float high_rr = 1 + (timer/temp_node->scheduled_service_time);
    while (temp_node)
    {
        response_ratio = 1 + (timer/temp_node->scheduled_service_time);
        if (response_ratio > high_rr)
        {
            high_rr = response_ratio;
            dq_node = temp_node;
            dq_prev_node = temp_prev_node;
        }
        temp_prev_node = temp_node;
        if (temp_node->next) temp_node = temp_node->next;
        else temp_node = NULL;
    }
    dq_prev_node->next = dq_node->next;
    dq_node->next = NULL;
    return dq_node;
}

/*notes:
 * waitpid()*/

/*** START OF SECTION MARKER ***/
/*** ADDITIONAL FUNCTION IMPLEMENTATIONS MAY BE ADDED HERE ***/
/*** END OF SECTION MARKER ***/
/*** END OF CODE; DO NOT ADD MATERIAL BEYOND THIS POINT ***/