#include <pthread.h>
#include <cstdlib>
#include "socketutils.hpp"
#include "proxy.hpp"

const char * port = "12345";
int main() {
  Proxy * proxy = new Proxy(port);
  proxy->run();
  return EXIT_SUCCESS;
}