#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/signal.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <string.h>

#define BUF_SIZE 256
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

    // get shm
    if ((shm_d = shm_open(SHARED_MEM_NAME,
                          O_RDWR | O_CREAT ,
                          S_IRUSR | S_IWUSR)) == -1)
    {

        perror("shm_open");
        exit(1);
    }

    // map shared memory to process virtual space
    shared_mem = (char *)mmap(NULL, BUF_SIZE,
                          PROT_READ | PROT_WRITE,
                          MAP_SHARED, shm_d, 0);

    while (1)
    {
        if (strlen(shared_mem) > 0)
        {
            printf("message received: %s\n", shared_mem);
            break;
        }
    }
    munmap(shared_mem, BUF_SIZE);
    shm_unlink(SHARED_MEM_NAME);
    return 0;
}