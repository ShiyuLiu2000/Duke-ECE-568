#ifndef SOCKETUTILS_H
#define SOCKETUTILS_H
#include <arpa/inet.h>  // for inet_ntop
#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <string>
#include <sys/types.h>
#include <netinet/in.h>

inline bool isEmptyInput(const std::string & str) {
    return str == "" || str == "\r" || str == "\n" || str == "\r\n";
}
// start server on specified port, and return socket file descriptor
inline int startServer(const char * portNum) {
  const char * hostName = NULL;
  int port = atoi(portNum);
  int status;
  int socket_fd;  // socket file descriptor
  struct addrinfo hints;  // provide hints about the type of socket you are supporting, pass NULLL for not care
  struct addrinfo * res;

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;      // use IPv4 or IPv6
  hints.ai_socktype = SOCK_STREAM;  // tcp stream sockets
  hints.ai_flags = AI_PASSIVE;      // let protocol decide ip

  std::string portStr = std::to_string(port);  // getaddrinfo reqiures the service as string
  status = getaddrinfo(hostName, portNum, &hints, &res);
  if (status != 0) {
    std::cerr << "Error: cannot get address info for Host: " + std::string(hostName) << std::endl;
    std::cerr << "  {" << hostName << " ," << port << "}" << std::endl;
    return -1;
  }
   if (strcmp(portNum, "") == 0) {
    struct sockaddr_in * addr_in = (struct sockaddr_in *)(res->ai_addr);
    addr_in->sin_port = 0;
  }
  // create a socket
  socket_fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
  if (socket_fd == -1) {
    std::cerr << "Error: cannot create socket" << std::endl;
    std::cerr << "  {" << hostName << "," << port << "}" << std::endl;
    freeaddrinfo(res);  // free the memory allocated by getaddrinfo
    return -1;
  }

  // allow port reuse if waiting in TIME_WAIT (part of tcp protocol, considered in the context of SO_REUSEADDR)
  int yes = 1;
  status = setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
  if (status == -1) {
    std::cerr << "Error: cannot set socket options" << std::endl;
    freeaddrinfo(res);
    close(socket_fd);
    return -1;
  }

  // bind socket to port
  status = bind(socket_fd, res->ai_addr, res->ai_addrlen);
  if (status == -1) {
    std::cerr << "Error: cannot bind socket" << std::endl;
    std::cerr << "  (" << hostName << "," << port << "}" << std::endl;
    freeaddrinfo(res);
    close(socket_fd);
    return -1;
  }

  // listen for incoming connections
  status = listen(socket_fd, 100);  // backlog of 100 connections
  if (status == -1) {
    std::cerr << "Error: cannot listen on socket" << std::endl;
    std::cerr << "  (" << hostName << "," << port << ")" << std::endl;
    freeaddrinfo(res);
    close(socket_fd);
    return -1;
  }

  freeaddrinfo(res);
  return socket_fd;
}

// accept a client connection on the given socekt file descriptor
// server_fd: file descriptor of server socket listening for connections
// return file descriptor for client connection, or -1 if error occurred
inline int acceptClientConnection(int server_fd, std::string & client_ip) {
  struct sockaddr_storage client_addr;
  socklen_t socket_addr_len = sizeof(client_addr);

  int client_connection_fd;
  client_connection_fd =
      accept(server_fd, (struct sockaddr *)&client_addr, &socket_addr_len);
  if (client_connection_fd == -1) {
    std::cerr << "Error: cannot accept connection on socket" << std::endl;
    return -1;
  }
  struct sockaddr_in * s = (struct sockaddr_in *)&client_addr;
  client_ip =  inet_ntoa(s->sin_addr);
  //std::cout << "Connection accepted from " << client_ip << std::endl;
  return client_connection_fd;
}

// build a client socket that can connect to server
// hostname: hostname or ip address of server
// return socket descriptor of connected client socket, or -1 if error occurred
inline int buildClient(const char * host, const char * port) {
  //const std::string hostname(host);
  //std::cout << "Verifying hostName: " << host << std::endl;
  struct addrinfo hints;
  struct addrinfo * res;
  //struct addrinfo * serverInfo;
  int status;
  int socket_fd; 

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;      // use IPv4 or IPv6
  hints.ai_socktype = SOCK_STREAM;  // tcp stream sockets

  status = getaddrinfo(host, port, &hints, &res);
  if (status != 0) {
    fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
    std::cerr << "Error: cannot get address info for host: " << host << std::endl;
    return -1;
  }

  // loop through all the address and bind to the first we can
  //for (serverInfo = res; serverInfo != NULL; serverInfo = serverInfo->ai_next) {
    // create a socket with the server info
    socket_fd =
        socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (socket_fd == -1) {
      // socket creation fail maybe due to non-recoverable error for this addr info
      // such as address family not supported by the system
      // Thus we try next address insteadof re-creating the socket for the same address
      return socket_fd;
    }

    // try to connect the socket
    status = connect(socket_fd, res->ai_addr, res->ai_addrlen);
    if (status == -1) {
      std::cerr << "Error: cannot create socket" << "  {" << host << "," << port << "}" << std::endl;
      close(socket_fd);
      return -1;
    }
    //std::cout << "Successfully connected to server" << std::endl;
     freeaddrinfo(res);
     return socket_fd;
}
#endif