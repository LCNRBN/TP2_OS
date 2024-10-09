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

void* malloc_3is(size_t size){
    void *currentBreak = sbrk(size + sizeof(HEADER) + sizeof(long));
    if (currentBreak == (void *) -1){
        if (errno==ENOMEM){
            printf("Allocation error/Not enough space");
        }
    }
    return currentBreak;
}


int main(){
    printf("size of HEADER : %ld\n", sizeof(HEADER));
    void *currentBreak = sbrk(0);
    currentBreak = sbrk(10);
    printf("First adress : %p\n", currentBreak);
    currentBreak = malloc_3is(10);
    printf("Adress before malloc 10 ; %p\n", currentBreak);
    currentBreak = malloc_3is(100);
    printf("Adress before malloc 100 ; %p\n", currentBreak);
    currentBreak = sbrk(0);
    printf("Last adress : %p\n", currentBreak);
}


/*void free_3is(void *ptr) {
    if (ptr == NULL){
        return;
    }
    HEADER* head = 
}*/