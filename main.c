#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

typedef struct HEADER_TAG {
    struct HEADER_TAG * ptr_next;
    size_t bloc_size;
    long magic_number; /* 0x0123456789ABCDEFL;*/ 
}HEADER;

void* malloc_3is(size_t size){
    int *currentBreak = sbrk(size + sizeof(HEADER) + sizeof(long));
    if (currentBreak == (void *) -1){
        if (errno==ENOMEM){
            printf("Allocation error/Not enough space");
        }
    }
    return NULL;
}

void free_3is(void *ptr) {

}

int main(){
    malloc_3is(10);
}