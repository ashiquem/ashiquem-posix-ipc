#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/signal.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <string.h>

#define BUF_SIZE 256
#define NUMBER_ITERATIONS 10
#define SHARED_MEM_NAME "/shared-mem-writer" // must start with '/'

// cleanup
static void _sig_handler(int signo)
{
    if (signo == SIGTERM || signo == SIGINT)
    {
        shm_unlink(SHARED_MEM_NAME);
        printf("removed %s\n", SHARED_MEM_NAME);
        exit(signo);
    }
}

int main(int argc, char **argv)
{
    char *shared_mem;
    int shm_d;
    
    shm_unlink(SHARED_MEM_NAME); // unlink if exists

    if ((shm_d = shm_open(SHARED_MEM_NAME,
                          O_RDWR | O_CREAT,
                          S_IRUSR | S_IWUSR)) == -1)
    {

        perror("shm_open");
        exit(1);
    }

    if (ftruncate(shm_d, BUF_SIZE) == -1)
    {

        perror("ftruncate");
        exit(1);
    }
    // map shared memory to process virtual space
    shared_mem = (char *)mmap(NULL, BUF_SIZE,
                              PROT_READ | PROT_WRITE,
                              MAP_SHARED, shm_d, 0);
    char message[BUF_SIZE];

    for (int i = 0; i < NUMBER_ITERATIONS; i++)
    {
        sprintf(message, "hello world %i\n", i);
        printf("writing: %s", message);
        strncpy(shared_mem, message, BUF_SIZE);
    }

    return 0;
}