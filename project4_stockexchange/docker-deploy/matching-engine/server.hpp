#ifndef SERVER_HPP
#define SERVER_HPP

#include <arpa/inet.h>
#include <assert.h>
#include <netdb.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

#include <atomic>
#include <cstring>
#include <iostream>
#include <string>

#include "database.hpp"

using namespace std;
using namespace pqxx;

class Server {
 public:
  int status;
  int socket_fd;
  struct addrinfo host_info;
  struct addrinfo * host_info_list;
  const char * port;

  bool running;
  // connection * C;

  // default constructor
  Server() : socket_fd(-1), host_info_list(nullptr) {}

  Server(const char * port);
  ~Server();

  // develop based on my code from ECE 650
  void createHost(const char * port);
  void createClient(const char * hostname, const char * port);
  int acceptConnection();

  void runServer(connection * C);
  // void stopServer();
  // this function will read the request from client and send the response 
  static void * handleClient(void * arg); 

  // static void * handle(void * info);
  // void connectDatabase(); -> write in database.hpp
};

struct client_fd_c {
    int client_fd;
    pqxx::connection * C;
};


#endif