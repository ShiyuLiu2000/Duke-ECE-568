#include "server.hpp"

int main(int argc, char * argv[]) {
  connection * C = connectDatabase();
  dropTables(C);
  createTables(C);
  const char * port = "12345";
  Server server(port);

  try {
    std::cout << "Starting server on port " << port << std::endl;
    server.runServer(C);
    std::cout << "Stop running server. " << std::endl;
    server.~Server();
    //server.stopServer();
  }
  catch (const std::exception & e) {
    std::cerr << "Exception: " << e.what() << std::endl;
    return EXIT_FAILURE;
  }
  if (C) {
    C->disconnect();
    delete C;
  }
  return EXIT_SUCCESS;
}
