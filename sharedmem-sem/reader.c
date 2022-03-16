#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/signal.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <semaphore.h>
#include <string.h>

#define BUF_SIZE 256
#define SHARED_MEM_NAME "/shared-mem-writer" // must start with '/'
#define SEM_WRITER "/sem-writer"             // must start with '/'
#define SEM_READER "/sem-reader"             // must start with '/'

// cleanup
static void _sig_handler(int signo)
{
    if (signo == SIGTERM || signo == SIGINT)
    {
        shm_unlink(SHARED_MEM_NAME);
        sem_unlink(SEM_READER);
        sem_unlink(SEM_WRITER);
        printf("removed %s\n", SHARED_MEM_NAME);
        exit(signo);
    }
}

int main(int argc, char **argv)
{
    char shared_mem[BUF_SIZE];
    sem_t *writer_sem, *reader_sem;
    int shm_d;

    // get shm
    shm_unlink(SHARED_MEM_NAME); // unlink if exists
    
    if ((shm_d = shm_open(SHARED_MEM_NAME,
                          O_RDONLY | O_CREAT,
                          S_IRUSR)) == -1)
    {

        perror("shm_open");
        exit(1);
    }

    // map shared memory to process virtual space
    // shared_mem = mmap(NULL, BUF_SIZE,
    //                           PROT_READ,
    //                           MAP_SHARED, shm_d, 0);

    // setup semaphores to synchronize

    // remove if exits
    sem_unlink(SEM_READER);
    sem_unlink(SEM_WRITER);

    reader_sem = sem_open(SEM_READER, O_CREAT, 0660, 1);
    writer_sem = sem_open(SEM_WRITER, O_CREAT, 0660, 0);

    while (1)
    {
        sem_wait(writer_sem); // wait till theres' something to consume
        // read current contents of shared segment
        read(shm_d, shared_mem, BUF_SIZE);
        if (strlen(shared_mem) > 0)
        {
            printf("message received: %s\n", (char *)shared_mem);
        }
        sem_post(reader_sem); // alert producer youre ready to consume again
    }
    // wont reach
    return 0;
}