# SRC Example7.
A Very complex example of using SRC as server and client  using threads that **not work**.

- Describes an interface EX1 in a SRC file.

- Compile it and create stub and skeleton.

- Describes an interface EX2 in a SRC file.

- Compile it and create stub and skeleton.

- Create a server1 with studbof interface EX1 and skeleton of EX2.
  
- Create a server2 with stub of interface EX2 and skeleton of EX1. 

- Run and verify.

## Describes an interface

  ex1.src describe class EX1.
  
  - EX1, include 3 method:
     - one one-way method with simple parameters.
     - one one-way method with array parameters.
     - one two-way method that return number of call receive.

  ex2.src describe class EX2.

  - EX2, include 3 method:
     - one two-way method with simple parameters.
     - one two-way method with array parameters.
     - one two-way method that return number of call receive.
  
## Compile

  make clean, clean. 
  
  make server1, make the server1.

  make server2, make the server2.
  
  make or make todo: make clean server1 server2.

## Server1

server1.c include the code for the server.

- Create a thread (runServer):

  - Open a local Address1. (SRC_Open)
  - Infinite loop over SRC
  - End SRC.

- In the main thread Create a poll with standard input

- When receive something from SRC (EX1) attends it and  **make a call to a EX2 method**.

- When receive a text in the standard input  if it is "end" finish, **if not make a call to a EX2 method**.


## Server2

server2.c include the code for the server.

- Create a thread (runServer):

  - Open a local Address1. (SRC_Open)
  - Infinite loop over SRC
  - End SRC.

- In the main thread Create a poll with standard input

- When receive something from SRC (EX1) attends it and  **make a call to a EX1 method**.

- When receive a text in the standard input  if it is "end" finish, **if not make a call to a EX1 method**.

## Test.

Run server1 and server2 in diferent terminals.

Enter something in each terminal.


> [!IMPORTANT]
> You can observe it work in a direction but not in the opposite.

> [!IMPORTANT]
> Remember SRC is not thread-safe.

> [!IMPORTANT]
> Use on one-way methods in these scenarios
