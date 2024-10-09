#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

#define MAGIC_NUMBER 0x0123456789ABCDEFL
typedef struct HEADER_TAG {
    struct HEADER_TAG * ptr_next;
    size_t bloc_size;
    long magic_number; /* 0x0123456789ABCDEFL;*/ 
}HEADER;

HEADER *head_list_free = NULL;  

void* malloc_3is(size_t size){
    void *currentBreak = sbrk(size + sizeof(HEADER) + sizeof(long));
    if (currentBreak == (void *) -1){
        if (errno==ENOMEM){
            printf("Allocation error/Not enough space");
        }
    }
    return currentBreak + sizeof(HEADER);
}

void free_3is(void *ptr) {
    if (ptr == NULL){
        return;
    }
    HEADER *block = ptr - sizeof(HEADER);
    block->ptr_next = head_list_free; 
    head_list_free = block;    
}

int main(){
    printf("size of HEADER : %ld\n", sizeof(HEADER));
    void *currentBreak = sbrk(0);
    currentBreak = sbrk(10);
    printf("First adress : %p\n", currentBreak);
    void * p1 = malloc_3is(10);
    printf("Adress of memory block before malloc 10 ; %p\n", p1);
    void * p2 = malloc_3is(100);
    printf("Adress of memory block before malloc 100 ; %p\n", p2);
    void * p3 = sbrk(0);
    printf("Last adress : %p\n", p3);

    free_3is(p1);
    free_3is(p2);
    free_3is(p3);
    HEADER *current = head_list_free;
    printf("List of free blocks:\n");
    while (current != NULL) {
        printf("Free block  at address %p\n", (void *)current);
        current = current->ptr_next;
    }
}