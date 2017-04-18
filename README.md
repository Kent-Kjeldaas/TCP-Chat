# A Simple Chat Program With Client and Server

This folder contains the associated code for a server and clients. It is using the Transmission Control Protocol. One server is needed and can run multiple clients.

## Summary

The project consists of a server and a client class. The server can easily have tens of clients. Each client is able to send and recieve messages and gets assigned a unique id for each.

**Feel free to use the code as your own! If you have any questions regarding the project, don't hesitate to contact!**

### Client Class

The client class is using socket, ip, port and buffer size can be changed in the settings. If a known error is detected it gets displayed. 
The client will try to connect to the server. The client gets an id so the server will know which client is sending which message. 
A new thread is started and it starts looking for messages to display, and enables the user to send messages. If something failes it cleans up and terminates.

### Server Class

The server is using Winsock, looking for clients, and then create a thread for each client. The thread running is the server-chat, and runs functions which enables communication for the clients. 
Then open the server to accept various clients forward the max number of clients. Each client gets a thread running function "client process".
This function sends first message to all clients connected. The message tells you that a new client has connected and how many are online. 
Then it waits for new messages Recv (), and sends them to all clients when the message is received (or error message).
