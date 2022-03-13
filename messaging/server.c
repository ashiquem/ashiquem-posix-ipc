#include <mqueue.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/signal.h>

#define QUEUE_NAME "/server-queue" // must start with '/'
#define MAX_MESSAGES 10
#define MAX_MSG_SIZE 256
#define MSG_BUFFER_SIZE MAX_MSG_SIZE + 10

static mqd_t mq_server;

// cleanup
static void _sig_handler(int signo)
{
    if (signo == SIGTERM || signo == SIGINT)
    {
        mq_close(mq_server);
        mq_unlink(QUEUE_NAME);
        printf("removed %s\n", QUEUE_NAME);
        exit(signo);
    }
}

int main(int argc, char **argv)
{
    if (SIG_ERR == signal(SIGTERM, _sig_handler))
    {
        fprintf(stderr, "Unable to catch SIGTERM...exiting.\n");
        exit(EXIT_FAILURE);
    }

    if (SIG_ERR == signal(SIGINT, _sig_handler))
    {
        fprintf(stderr, "Unable to catch SIGINT...exiting.\n");
        exit(EXIT_FAILURE);
    }

    // des,name, flags, permissions, attributes
    int flags = O_RDONLY | O_CREAT; // server only reads, create new q if not exists
    int perms = S_IRUSR | S_IWUSR;
    struct mq_attr attr;

    attr.mq_flags = 0;
    attr.mq_maxmsg = MAX_MESSAGES;
    attr.mq_msgsize = MAX_MSG_SIZE;
    attr.mq_curmsgs = 0;
    mqd_t mq_client; // client q des

    if ((mq_server = mq_open(QUEUE_NAME, flags, perms, &attr)) == -1)
    {
        perror("Failed to create server queue");
        exit(EXIT_FAILURE);
    };
    printf("Created %s\n", QUEUE_NAME);

    int counter = 0;

    char in_buffer[MSG_BUFFER_SIZE]; // client msg q name
    char out_buffer[MSG_BUFFER_SIZE];

    while (1)
    {
        // read msg from own queue
        if (mq_receive(mq_server, in_buffer, MSG_BUFFER_SIZE, NULL) == -1)
        {
            perror("Failed to receive from queue");
            exit(EXIT_FAILURE);
        };

        // process received message
        sprintf(out_buffer, "hello from %d - count:%d", getpid(), counter);

        // connect to client queue
        if ((mq_client = mq_open(in_buffer, O_WRONLY)) == -1)
        {
            perror("Failed to connect to client queue");
            continue;
        };

        // send message to client queue
        if (mq_send(mq_client, out_buffer, strlen(out_buffer) + 1, 0) == -1)
        {
            perror("Failed to send msg to client queue");
            continue;
        };
        printf("Message sent to %s\n", in_buffer);
        counter++;
    }

    // wont reach
    return 0;
}