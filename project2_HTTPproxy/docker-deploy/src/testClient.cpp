#include <iostream>
#include <string>

#include "socketutils.hpp"

// test buildClient()
int main(void) {
  const char * port = "1235";  // use non-reserved port numbers above 1024
  const char * hostname = "localhost";

  std::cout << "Trying to start server on port " << port << std::endl;
  int server_fd = startServer(port);
  if (server_fd == -1) {
    std::cerr << "Server failed to start" << std::endl;
    return EXIT_FAILURE;
  }

  std::cout << "Server started successfully. Waiting for connections..." << std::endl;
  std::cout << "Building client to connect to " << hostname << std::endl;
  int client_fd = buildClient(hostname, port);
  if (client_fd == -1) {
    std::cerr << "Failed to connect client to server" << std::endl;
    close(server_fd);
    std::cout << "Server shutdown" << std::endl;
    return EXIT_FAILURE;
  }
  std::cout << "Successfully built client, and connected to server" << std::endl;

  close(client_fd);
  std::cout << "Client disconnected" << std::endl;
  close(server_fd);
  std::cout << "Server shutdown" << std::endl;

  return EXIT_SUCCESS;
}