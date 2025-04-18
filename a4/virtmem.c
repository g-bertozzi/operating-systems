/*
* Some starter code for CSC 360, Spring 2025, Assignment #4
*
* Prepared by: 
* Michael Zastre (University of Victoria) -- 2024
* 
* Modified for ease-of-use and marking by 
* Konrad Jasman (University of Victoria) -- 2025
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

/*
* Some compile-time constants.
*/
#define REPLACE_NONE 0
#define REPLACE_FIFO 1
#define REPLACE_LRU  2
#define REPLACE_CLOCK 3
#define REPLACE_OPTIMAL 4

#define TRUE 1
#define FALSE 0
#define PROGRESS_BAR_WIDTH 60
#define MAX_LINE_LEN 100

/*
* Some function prototypes to keep the compiler happy.
*/
int setup(void);
int teardown(void);
int output_report(void);
long resolve_address(long, int);
void error_resolve_address(long, int);

/*
* Variables used to keep track of the number of memory-system events
* that are simulated.
*/
int page_faults = 0;
int mem_refs    = 0;
int swap_outs   = 0;
int swap_ins    = 0;

/*
* Page-table information. You are permitted to modify this in order to
* implement schemes such as CLOCK. However, you are not required
* to do so.
*/
struct page_table_entry *page_table = NULL;
typedef struct page_table_entry {
    long page_num;// the logical page number this entry maps to
    int dirty; // set if page is modified since being loaded into memory
    int free; // indicates if frame is available or occupied

    struct page_table_entry *next; // for lists
    struct page_table_entry *prev; // for lists (LRU)
    int ref; // clock
} page_table_entry;



/*
* These global variables will be set in the main() function. The default
* values here are non-sensical, but it is safer to zero out a variable
* rather than trust to random data that might be stored in it -- this
* helps with debugging (i.e., eliminates a possible source of randomness
* in misbehaving programs).
*/
int size_of_frame = 0;  /* power of 2 */
int size_of_memory = 0; /* number of frames */
int page_replacement_scheme = REPLACE_NONE;


// fifo queue; store order of arrival of frame indexes
// use to access page by page_table[frame]
typedef struct {
    int *queue;
    int front;
    int rear;
    int count;
} fifo_queue_t;

// lru doubly linked list
typedef struct {
    page_table_entry *head;
    page_table_entry *tail;
} lru_list_t;

// clock struct
typedef struct {
    page_table_entry *clock_hand;
    int size;  // how many frames filled
} clock_list_t;

// global data structs for each policy
fifo_queue_t fifo;
lru_list_t lru;
clock_list_t clock_list;

// fifo functions
void fifo_enqueue(fifo_queue_t *fq, int frame) {
    fq->queue[fq->rear] = frame;
    fq->rear = (fq->rear + 1) % size_of_memory;
    fq->count++;
}
int fifo_dequeue(fifo_queue_t *fq) {
    int frame = fq->queue[fq->front];
    fq->front = (fq->front + 1) % size_of_memory;
    fq->count--;
    return frame;
}
// lru functions
void lru_append(page_table_entry *entry) {
    entry->next = lru.head;
    entry->prev = NULL;
    if (lru.head != NULL) lru.head->prev = entry;
    else lru.tail = entry;
    lru.head = entry;
}
void lru_move_to_front(page_table_entry *entry) {
    if (entry == lru.head) return;
    if (entry->prev) entry->prev->next = entry->next;
    if (entry->next) entry->next->prev = entry->prev;
    if (entry == lru.tail) lru.tail = entry->prev;
    entry->next = lru.head;
    entry->prev = NULL;
    if (lru.head) lru.head->prev = entry;
    lru.head = entry;
}
page_table_entry* lru_remove_tail() {
    page_table_entry *evict = lru.tail;
    if (!evict) return NULL;
    if (evict->prev) {
        evict->prev->next = NULL;
        lru.tail = evict->prev;
    } else {
        lru.head = lru.tail = NULL;
    }
    evict->next = evict->prev = NULL;
    return evict;
}

/* CLOCK replacement policy:
 * - Each frame has a reference bit (ref).
 * - On each memory access, set ref = 1 for the accessed page.
 * - When replacement is needed:
 *     - Check the frame pointed to by the clock hand.
 *     - If ref == 1: set ref = 0 and move the clock hand forward.
 *     - If ref == 0: evict this page.
 * - The clock hand moves in a circular fashion through the frame list.
*/

/*
* Function to convert a logical address into its corresponding 
* physical address. The value returned by this function is the
* physical address (or -1 if no physical address can exist for
* the logical address given the current page-allocation state.
*/
long resolve_address(long logical, int memwrite){
    int i;
    long page, frame;
    long offset;
    long mask = 0;
    long effective;

    /* Get the page and offset */
    page = (logical >> size_of_frame);

    for (i=0; i<size_of_frame; i++){
        mask = (mask << 1) | 1;
    }
    offset = logical & mask;

    // Try to find page in memory
    frame = -1;
    for (i = 0; i < size_of_memory; i++) {
        if (!page_table[i].free && page_table[i].page_num == page) {
            frame = i;
            if (memwrite) {
                page_table[frame].dirty = TRUE; /* Mark page dirty if written */
            }
            break;
        }
    }

    // page is in memory 
    if (frame != -1) {
        // TODO: logic for lru and clock updates
        if (page_replacement_scheme == REPLACE_LRU) {
            lru_move_to_front(&page_table[frame]);
        } else if (page_replacement_scheme == REPLACE_CLOCK) {
            page_table[frame].ref = 1;
        }
        
        effective = (frame << size_of_frame) | offset;
        return effective;
    }

    page_faults++; // page is not in memory

    // check if there are free frames in memory
    for (i = 0; i < size_of_memory; i++) {
        if (page_table[i].free) {
            frame = i;
            break;
        }
    }

    // no free frames -> choose victim
    if (frame == -1) {
        if (page_replacement_scheme == REPLACE_FIFO) {
            frame = fifo_dequeue(&fifo); // first in first out
            if (page_table[frame].dirty == TRUE) { // if read op -> inc swap_out
                swap_outs++;
            }
            page_table[frame].free = TRUE; // signal victim is longer in ram
        } 
        else if (page_replacement_scheme == REPLACE_LRU) {
            page_table_entry *evicted = lru_remove_tail();
            frame = evicted - page_table;
            if (evicted->dirty) {
                swap_outs++;
            }
            evicted->free = TRUE; // signal victim is longer in ram
        } else if (page_replacement_scheme == REPLACE_CLOCK) {
            while (1) {
                if (clock_list.clock_hand->ref == 0) {
                    if (clock_list.clock_hand->dirty) swap_outs++;
                    clock_list.clock_hand->free = TRUE;
                    frame = clock_list.clock_hand - page_table;
                    break;
                } else {
                    clock_list.clock_hand->ref = 0;
                    clock_list.clock_hand++;
                    if (clock_list.clock_hand >= &page_table[size_of_memory]) {
                        clock_list.clock_hand = &page_table[0]; // wrap around
                    }
                }
            }
        }
    }

    // free frame -> load into page
    if (frame != -1) {
        page_table[frame].page_num = page;
        page_table[frame].free = FALSE; // signal page is in RAM
        page_table[frame].dirty = memwrite ? TRUE : FALSE; // if write op-> dirty
        swap_ins++;

        if (page_replacement_scheme == REPLACE_FIFO) {
            fifo_enqueue(&fifo, frame);
        } else if (page_replacement_scheme == REPLACE_LRU) {
            lru_append(&page_table[frame]);
        } else if (page_replacement_scheme == REPLACE_CLOCK) {
            page_table[frame].ref = 1;
        }
        
        effective = (frame << size_of_frame) | offset;
        return effective;
    }

    // If no frame was found or created -> return error
    // TODO: remove testing
    // printf("Couldn't find available frame.\n");
    return -1;
}

/*
* Super-simple progress bar.
*/
void display_progress(int percent){
    int to_date = PROGRESS_BAR_WIDTH * percent / 100;
    static int last_to_date = 0;
    int i;

    if (last_to_date < to_date){
        last_to_date = to_date;
    } else {
        return;
    }

    printf("Progress [");
    for (i=0; i<to_date; i++){
        printf(".");
    }
    for (; i<PROGRESS_BAR_WIDTH; i++){
        printf(" ");
    }
    printf("] %3d%%", percent);
    printf("\r");
    fflush(stdout);
}

// TODO: intialize data structs for each policy
int setup(){
    int i;

    page_table = (struct page_table_entry *)malloc(
        sizeof(struct page_table_entry) * size_of_memory
    );

    if (page_table == NULL){
        fprintf(stderr,
            "Simulator error: cannot allocate memory for page table.\n");
        exit(1);
    }

    for (i=0; i<size_of_memory; i++){ // initalize all frames as free
        page_table[i].free = TRUE;
    }

    // TODO: allocate & initalize data structs
    if (page_replacement_scheme == REPLACE_FIFO) {
        fifo.queue = (int *)calloc(size_of_memory, sizeof(int));
        if (fifo.queue == NULL) {
            fprintf(stderr, "Memory allocation failed for FIFO queue.\n");
            exit(1);
        }
        fifo.front = fifo.rear = fifo.count = 0;
    } else if (page_replacement_scheme == REPLACE_LRU) {
        lru.head = NULL;
        lru.tail = NULL;
    } else if (page_replacement_scheme == REPLACE_CLOCK) {
        clock_list.clock_hand = &page_table[0]; // Start at beginning
        clock_list.size = size_of_memory;

    } // else if (page_replacement_schemed == optimal)

    return -1;
}

// TODO: clean up data structs- only using something speical for fifo?
int teardown(){
    if (fifo.queue != NULL) {
        free(fifo.queue);
        fifo.queue = NULL;
    }
    return -1;
}

/*
* Function to report an address resolution error.
*/
void error_resolve_address(long a, int l){
    fprintf(stderr, "\n");
    fprintf(stderr, 
        "Simulator error: cannot resolve address 0x%lx at line %d\n",
        a, l
    );
    exit(1);
}

/*
* Function to print a summary report after simulation.
*/
int output_report(){
    printf("\n");
    printf("Memory references: %d\n", mem_refs);
    printf("Page faults: %d\n", page_faults);
    printf("Swap ins: %d\n", swap_ins);
    printf("Swap outs: %d\n", swap_outs);

    return -1;
}

/*
* Main simulation function.
*/
int main(int argc, char **argv){
    int i;
    char *s;
    FILE *infile = NULL;
    char *infile_name = NULL;
    struct stat infile_stat;
    int line_num = 0;
    int infile_size = 0;
    char buffer[MAX_LINE_LEN];
    long addr;
    char addr_type;
    int is_write;
    int show_progress = FALSE;

    /* Process command-line arguments */
    for (i=1; i < argc; i++){
        if (strncmp(argv[i], "--replace=", 9) == 0){
            s = strstr(argv[i], "=") + 1;
            if (strcmp(s, "fifo") == 0){
                page_replacement_scheme = REPLACE_FIFO;
            } else if (strcmp(s, "lru") == 0){
                page_replacement_scheme = REPLACE_LRU;
            } else if (strcmp(s, "clock") == 0){
                page_replacement_scheme = REPLACE_CLOCK;
            } else if (strcmp(s, "optimal") == 0){
                page_replacement_scheme = REPLACE_OPTIMAL;
            } else {
                page_replacement_scheme = REPLACE_NONE;
            }
        } else if (strncmp(argv[i], "--file=", 7) == 0){
            infile_name = strstr(argv[i], "=") + 1;
        } else if (strncmp(argv[i], "--framesize=", 12) == 0){
            s = strstr(argv[i], "=") + 1;
            size_of_frame = atoi(s);
        } else if (strncmp(argv[i], "--numframes=", 12) == 0){
            s = strstr(argv[i], "=") + 1;
            size_of_memory = atoi(s);
        } else if (strcmp(argv[i], "--progress") == 0){
            show_progress = TRUE;
        }
    }

    /* Open file if specified */
    if (infile_name == NULL){
        infile = stdin;
    } else if (stat(infile_name, &infile_stat) == 0){
        infile_size = (int)(infile_stat.st_size);
        infile = fopen(infile_name, "r");  
    }

    /* Check for missing parameters */
    if (page_replacement_scheme == REPLACE_NONE ||
        size_of_frame <= 0 ||
        size_of_memory <= 0 ||
        infile == NULL)
    {
        fprintf(stderr, 
            "usage: %s --framesize=<m> --numframes=<n>", argv[0]);
        fprintf(stderr, 
            " --replace={fifo|lru|optimal} [--file=<filename>]\n");
        exit(1);
    }

    setup();

    /* Process each line of the input file */
    while (fgets(buffer, MAX_LINE_LEN-1, infile)){
        line_num++;
        if (strstr(buffer, ":")){
            sscanf(buffer, "%c: %lx", &addr_type, &addr);
            is_write = (addr_type == 'W') ? TRUE : FALSE;

            if (resolve_address(addr, is_write) == -1){
                error_resolve_address(addr, line_num);
            }
            mem_refs++;
        } 

        if (show_progress){
            display_progress(ftell(infile) * 100 / infile_size);
        }
    }

    teardown();
    output_report();
    fclose(infile);
    exit(0);
}
