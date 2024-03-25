#include <iostream>
#include <string>

#include "socketutils.hpp"

// test startServer() and acceptClientConnection(), with netcat
int main(void) {
  const char * port = "1234";  // use non-reserved port numbers above 1024
  std::string
      client_ip;  // pass it to acceptClientConnection, this will be filled by `accept`

  std::cout << "Trying to start server on port " << port << std::endl;
  int server_fd = startServer(port);
  if (server_fd == -1) {
    std::cerr << "Server failed to start" << std::endl;
    return EXIT_FAILURE;
  }

  std::cout << "Server started successfully. Waiting for connections..." << std::endl;

  int client_fd = acceptClientConnection(server_fd, client_ip);
  if (client_fd == -1) {
    std::cerr << "Failed to accept connection" << std::endl;
    return EXIT_FAILURE;
  }

  close(client_fd);
  std::cout << "Client disconnected" << std::endl;
  close(server_fd);
  std::cout << "Server shutdown" << std::endl;
  return EXIT_SUCCESS;
}