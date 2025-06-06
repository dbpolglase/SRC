# SRC Example5.
A simple example of using SRC using a broadcast address.

- Describes an interface in a SRC file.

- Compile it and create stub and skeleton.

- Just create a server for the stub and a client for the skeleton.

- Run and verify.

## Describes an interface

  example2.src describe 3 class EX1,EX2,EX3.
  
  - EX1, include 2 method:
     - one one-way method with simple parameters.
     - one one-way method with array parameters.

  - EX2, include 2 method:
     - one two-way method with simple parameters.
     - one two-way method with array parameters.

  - EX3, include 1 method to just return number of call receive.
  
## Compile

  make clean, clean. 
  
  make server, make the server.
  
  make client, make the client.
  
  make or make todo: make clean server and client.

## Server

server.c include the code for the server.

- Open a broadcast Address1. (SRC_Open)

- Open a broadcast Address2. (SRC_Open).

- Infinite loop waiting for request. (SRC_Loop).

- Implement methods for class EX1,EX2 and EX3 with C functions.


## Client

client.c include the code for the client.

Open any port as sender. (SRC_Open).

Create Address1 to server by hand.

Create Address2 to server using SRC (GetNormalAddress).

Ask for how number of calls to do.

Make request for EX1 class to Address1.

Make request for EX2 class to Address2.


## Test.

Run make execute or 

Run server and client by hand.

> [!IMPORTANT]
> Remember the firewall.

> [!TIP]
> Experiment using running several server process in one o more hosts.

> [!TIP]
> You also can run several client process, that's the normal.
