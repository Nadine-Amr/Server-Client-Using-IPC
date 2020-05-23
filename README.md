# Server-Client Problem Using IPC
Server-Client Problem Using Inter-Process Communication (IPC)

This repository simulates a server and a client processes communicating together. The client sends a string to the server, which in turn accepts it, converts lower case letters to upper case ones and vise versa, then sends it back to the client. Multiple clients can attempt to communicate with the server simultaneously.

The processes communicate through inter-process-communication (IPC). Two approaches are implemented:

    Using message queues
    Using shared memory and semaphores

It is assumed that there is only one server and which always TERMINATES LAST; after all clients terminate.
