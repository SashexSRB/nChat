# NERVChat - Client-Server Chat Application

**NERVChat** is a simple client-server chat application built using **Qt 6** and **TCP sockets**. It allows users to register, log in, and chat in real-time. The client and server are separate applications, both written in C++ using Qt, and they communicate via a TCP connection.

## Features
- **User authentication** (Login & Registration)
- **Real-time chat** with a server using TCP sockets
- **Separate client and server**: The server handles connections from multiple clients, and the client connects to the server for messaging
- **Qt GUI** for both client (user interface) and server interaction

## Project Structure

### Client
- The client application allows the user to register, log in, and chat with others.
- Built using **Qt 6** with **QTcpSocket** for communication.
- The client UI is designed using **Qt Designer**.
- The `QStackedWidget` is used to switch between the login screen and the main chat interface.

### Server
- The server application listens for incoming client connections, processes login requests, and forwards messages between clients.
- Also built with **Qt 6** and **QTcpServer**.

## Requirements

- **Qt 6.0** or later
- **OpenSSL** (for secure connections, if needed)
- A C++17 compatible compiler (QMake6)
- **QtCreator**

### Dependencies:
- **QtCore**
- **QtNetwork**
- **OpenSSL** (optional, for secure communication)

## Installation

### 1. Clone the repository

```bash
git clone https://github.com/your-username/NERVChat.git
cd NERVChat
```
### 2. Set up the Server
- Build it using the QtCreator
  
### 3. Set up the Client
- Build it using the QtCreator

## Usage

### Starting the Server
- Navigate to the build directory using cd, then just execute it in shell using ./server. It will listen for incoming connections on port 1644.

### Starting the Client
- Once the server is running, you can start the Client.
- Enter your username and password on the login screen/Register a new account.
- If the login successful, you'll be switched to the chat interface.
- Multiple clients can be run simultaneously, and they will be able to communicate via the server.
