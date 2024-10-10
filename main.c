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

// | header | memory block | magic number |

void* malloc_3is(size_t size){
    HEADER *prev = NULL;
    HEADER *current = head_list_free;
    while (current != NULL) {
        if (current->bloc_size >= size) {
            if (prev == NULL) {
                head_list_free = current->ptr_next;
            } else {
                prev->ptr_next = current->ptr_next;
            }
            current->ptr_next = NULL;  
            return (void *)(current + 1); 
        }
        prev = current;
        current = current->ptr_next;
    }
    HEADER* block = sbrk(size + sizeof(HEADER) + sizeof(long));

    if (block == (void *) -1){
        if (errno==ENOMEM){
            printf("Allocation error/Not enough space");
        }
        return NULL;
    }
    
    block->bloc_size = size;
    block->magic_number = MAGIC_NUMBER;
    //printf("block %p, block->magic_number %ld\n", block, block->magic_number);
    long *magic_block_end = (long*)(block + 1 + block->bloc_size); //pointer arithmetic
    *magic_block_end = MAGIC_NUMBER;
    printf("magic_block_end %p, *magic_block_end %ld\n", magic_block_end, *magic_block_end);
    return block + 1;
}

void free_3is(void *ptr) {
    if (ptr == NULL){
        return;
    }
    HEADER *block = ptr - sizeof(HEADER);
    block->ptr_next = head_list_free; 
    head_list_free = block;    
}

int check_malloc(void *ptr){
    if(ptr == NULL){
        return -1;
    }

    HEADER* block = ptr - sizeof(HEADER);

    if(block->magic_number != MAGIC_NUMBER){
        printf("memory overflow begin\n");
        return -1;
    }

    long *magic_block_end = (long*)(block + 1 + block->bloc_size);
    printf("magic_block_end %p, *magic_block_end %ld\n", magic_block_end, *magic_block_end);
    if(*magic_block_end != MAGIC_NUMBER){
        printf("memory overflow end\n");
        return -1;
    }

    return 0;
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

    if(check_malloc(p1) == 0){
        printf("no memory overflow\n");
    }
    if(check_malloc(p2) == 0){
        printf("no memory overflow\n");
    }

    free_3is(p1);
    free_3is(p2);

    void * p4 = malloc_3is(8);
    HEADER *current = head_list_free;
    printf("List of free blocks:\n");
    while (current != NULL) {
        printf("Free block  at address %p\n", (void *)current);
        current = current->ptr_next;
    }
    free_3is(p4);
}