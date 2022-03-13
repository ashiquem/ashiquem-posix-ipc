#include <mqueue.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/signal.h>

#define SERVER_QUEUE_NAME "/server-queue" // must start with '/'
#define MAX_MESSAGES 10
#define MAX_MSG_SIZE 256
#define MSG_BUFFER_SIZE MAX_MSG_SIZE + 10

static mqd_t mq_client;
char client_queue_name[64];

// cleanup
static void _sig_handler(int signo)
{
    if (signo == SIGTERM || signo == SIGINT)
    {
        mq_close(mq_client);
        mq_unlink(client_queue_name);
        printf("removed %s\n", client_queue_name);
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
    int flags = O_RDONLY | O_CREAT; // only reads, create new q if not exists
    int perms = S_IRUSR | S_IWUSR;
    struct mq_attr attr;

    attr.mq_flags = 0;
    attr.mq_maxmsg = MAX_MESSAGES;
    attr.mq_msgsize = MAX_MSG_SIZE;
    attr.mq_curmsgs = 0;
    mqd_t mq_server; // serve q des

    sprintf(client_queue_name, "/client-queue-%d", getpid());

    if ((mq_client = mq_open(client_queue_name, flags, perms, &attr)) == -1)
    {
        perror("Failed to create client queue");
        exit(EXIT_FAILURE);
    };
    printf("Created %s\n", client_queue_name);

    if ((mq_server = mq_open(SERVER_QUEUE_NAME, O_WRONLY)) == -1)
    {
        perror("Failed to connect to server queue");
        exit(EXIT_FAILURE);
    };

    char buffer[MSG_BUFFER_SIZE];

    char temp_buf[10];

    printf("Hit enter to send msg\n");
    while (fgets(temp_buf, 2, stdin))
    {
        // send msg to server queue
        if (mq_send(mq_server, client_queue_name, strlen(client_queue_name) + 1, 0) == -1)
        {
            perror("Failed to send to server queue");
            continue;
        };

        // receive message from own queue
        if (mq_receive(mq_client, buffer, MSG_BUFFER_SIZE, NULL) == -1)
        {
            perror("Failed to receive message from client queue");
            exit(EXIT_FAILURE);
        };

        printf("message received: %s\n", buffer);
    }

    // wont reach
    return 0;
}