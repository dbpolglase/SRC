# SRC Example1.
A simple example of using SRC.

- Describes an interface in a SRC file.

- Compile it and create stub and skeleton.

- Just create a server for the stub and a client for the skeleton.

- Run and verify.

## Describes an interface

  example1.src describe 3 class EX1,EX2,EX3.
  
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

Open the port. (SRC_Open)
Infinite loop waiting for request. (SRC_Loop).
Implement methods for class EX1,EX2 and EX3 with C functions.

## Client

clien.c include the code for the client.

Open any port as sender. (SRC_Open).

Create Address to server in this example by hand.

Ask for how number of calls to do.

Just do it.

## Test.

run make execute.

Or just run server and run client.

> [!TIP]
> You also can run several client process that's normal.

> [!TIP]
> You also can run several server process (SRC use SO_REUSE) just try what happen.

