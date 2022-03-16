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
#define NUMBER_ITERATIONS 10
#define SHARED_MEM_NAME "/shared-mem-writer" // must start with '/'
#define SEM_WRITER "/sem-writer"             // must start with '/'
#define SEM_READER "/sem-reader"             // must start with '/'

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
    sem_t *writer_sem, *reader_sem;
    int shm_d;

    if ((shm_d = shm_open(SHARED_MEM_NAME,
                          O_RDWR,
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
    // shared_mem = (char *)mmap(NULL, BUF_SIZE,
    //                           PROT_READ | PROT_WRITE,
    //                           MAP_SHARED, shm_d, 0);

    // close(shm_d);

    char message[BUF_SIZE];

    reader_sem = sem_open(SEM_READER, 0);
    writer_sem = sem_open(SEM_WRITER, 0);

    for (int i = 0; i < NUMBER_ITERATIONS; i++)
    {
        sem_wait(reader_sem); // wait till theres someone to consume
        sprintf(message, "hello world %i - from %i\n", i, getpid());
        // strncpy(shared_mem, message, BUF_SIZE);

        // if (msync(shared_mem, BUF_SIZE, MS_SYNC) < 0)
        // {
        //     perror("failed msync");
        //     exit(EXIT_FAILURE);
        // };
        write(shm_d, message, BUF_SIZE);
        printf("writing: %s\n", message);
        sem_post(writer_sem); // notify done producing ready to consume
    }

    sem_close(reader_sem);
    sem_close(writer_sem);
    return 0;
}