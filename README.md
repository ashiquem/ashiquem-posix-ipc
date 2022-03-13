# posix-ipc

## messaging

server process: 
- creates a new message queue
- expects client queue name in received message
- sends message to client queue

```
gcc -Wall server.c -o server -lrt
./server
```

client process:
- creates a new message queue
- sends queue name to server process
- receives message in own queue

```
gcc -Wall client.c -o client -lrt
./client
```