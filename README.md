# Dorm Reservation System

A multi-client dorm room reservation system built in C++ using raw TCP sockets. Features a multi threaded server handling multiple concurrent clients, a custom client program and a simple text based protocol for listing, reserving, canceling and checking room status.

## Features

- TCP client-server architecture built from raw POSIX sockets
- Multi threaded server supporting multiple concurrent clients
- Thread safe room data using a mutex
- Simple text based protocol: LIST, RESERVE, CANCEL, STATUS

## Compiling

Build both the server and client using the included Makefile: 

    make

This produces two executables: `server` and `client`. To remove them:

    make clean


## Usage

1. Start the server in one terminal:

       ./server

   The server listens on `127.0.0.1:8080` and prints room availability and connection logs as clients connect.

2. In a separate terminal, start a client:

       ./client

   The client connects to the server, then prompts for your student ID before you can send commands.

3. Available commands:

   - `LIST` — list all rooms with floor, type and availability
   - `RESERVE <room_id>` — reserve a room under your student ID
   - `CANCEL <room_id>` — cancel your reservation on a room
   - `STATUS <room_id>` — check a single room's booking status
   - `exit` — disconnect and close the client

   Multiple clients can connect to the same server at once; room data is shared and protected with a mutex.


## Protocol

The client and server communicate over plain text messages sent through TCP.

