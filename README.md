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