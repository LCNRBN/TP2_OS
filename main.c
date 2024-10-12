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

void split_block(HEADER *block, size_t size) {
    size_t remaining_size = block->bloc_size - (size + sizeof(HEADER) + sizeof(long));
    if (remaining_size > sizeof(HEADER) + sizeof(long)) {
        HEADER *new_block = block + 1 + size + sizeof(long);

        new_block->bloc_size = remaining_size;
        new_block->magic_number = MAGIC_NUMBER;

        new_block->ptr_next = block->ptr_next;
        block->ptr_next = new_block;
        block->bloc_size = size;
    }
}

void* malloc_3is(size_t size){
    HEADER *prev = NULL;
    HEADER *current = head_list_free;
    while (current != NULL) {
        if (current->bloc_size >= size + sizeof(HEADER) +sizeof(long)) {
            split_block(current, size);
            if (prev == NULL) {
                head_list_free = current->ptr_next;
            } else {
                prev->ptr_next = current->ptr_next;
            }
            current->ptr_next = NULL;  
            current->magic_number=MAGIC_NUMBER;
            long *magic_block_end = (long*)(current + 1 + current->bloc_size);
            *magic_block_end = MAGIC_NUMBER;
            return (current + 1); 
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
    long *magic_block_end = block + 1 + block->bloc_size; //pointer arithmetic
    *magic_block_end = MAGIC_NUMBER;
    printf("magic_block_end %p, *magic_block_end %ld\n", magic_block_end, *magic_block_end);
    return block + 1;
}

void free_3is(void *ptr) {
     if (ptr == NULL){
        return;
    }
    HEADER *prev = NULL;
    HEADER *current = head_list_free;
    HEADER *block = ptr - sizeof(HEADER);
   
    while(block < current && current != NULL){ //sorting out
        prev = current;
        current = current->ptr_next;
    }

    block->ptr_next = current; 

    if (prev == NULL){
        head_list_free = block;
    } 
    else{
        prev->ptr_next = block;
    }

//  | header 1 | memory block 1 | magic number 1 | inserted header 2 | inserted memory block 2 | inserted magic number 2 | header 3 | memory block 3 | magic number 3 |
    
    if (block->ptr_next != NULL && (block + 1) + block->bloc_size + sizeof(long) == block->ptr_next){ //contiguous addresses with following block 3
        long *magic_number_position = (block + 1) + block->bloc_size;
        *magic_number_position = NULL; //delete inserted magic number 2 (keep magic number 3)
        block->bloc_size += sizeof(HEADER) + block->ptr_next->bloc_size + sizeof(long);
        block->ptr_next = block->ptr_next->ptr_next; //merge 2 and 3
    }

    if (prev != NULL && (prev + 1) + prev->bloc_size + sizeof(long) == block){ //contiguous addresses with previous block 1
        prev->bloc_size += sizeof(HEADER) + block->bloc_size + sizeof(long);
        long *magic_number_position = (prev + 1) + prev->bloc_size;
        *magic_number_position = NULL; //delete magic number 1 (keep inserted magic number 2)
        prev->bloc_size += sizeof(HEADER) + block->bloc_size + sizeof(long);
        prev->ptr_next = block->ptr_next; //merge 1 and 2

            if (prev->ptr_next != NULL && (prev + 1) + prev->bloc_size + sizeof(long) == prev->ptr_next){ //contiguous addresses with both previous and following blocks 1, 3
            prev->bloc_size += sizeof(HEADER) + prev->ptr_next->bloc_size + sizeof(long);
            magic_number_position = (prev + 1) + prev->bloc_size;
            *magic_number_position = NULL; //delete magic number 2 (keep magic number 3)
            prev->ptr_next = prev->ptr_next->ptr_next; //merge (merge 1 and 2) and 3
        }
    }
    //Lucian : it is important to delete/reset some magic number because of the memory overflow checks in check_malloc()
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
    printf("======= Block malloc_3is Test =======\n");
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

    printf("======= Block check_malloc Test =======\n");
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
    free_3is(p3);

    printf("======= Block merge Test =======\n");
    void * p5 = malloc_3is(21);
    void * p6 = malloc_3is(11);
    void * p7 = malloc_3is(11);
    free_3is(p6);
    free_3is(p7);
    
    current = head_list_free;
    printf("free blocks list after free_3is of Blocks 6 and 7 :\n");
    while (current != NULL) {
        printf("free block at %p, size : %lu\n", current, current->bloc_size);
        current = current->ptr_next;
    }
    free_3is(p5);
    // several issues here
    current = head_list_free;
    printf("free blocks list after free_3is and merge of all 3 blocks :\n");
    while (current != NULL) {
        printf("free block at %p, size : %lu\n", (void *)current, current->bloc_size);
        current = current->ptr_next;
    }

}