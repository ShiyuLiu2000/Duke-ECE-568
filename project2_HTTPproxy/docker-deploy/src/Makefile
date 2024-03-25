# Declare all targets
PROGS = main

# Define flags
CFLAGS = -g -lpthread -Wall #-Werror -pedantic

all: $(PROGS)

# Target for main executable
main: main.cpp proxy.cpp proxy.hpp request.cpp request.hpp response.cpp response.hpp LRUCache.hpp socketutils.hpp ClientContext.hpp time.hpp
	g++ $(CFLAGS) -o main main.cpp proxy.cpp request.cpp response.cpp

.PHONY: clean

clean:
	rm -f *.o *~ $(PROGS)

# target for test connection executable
#testConnection: testConnection.cpp socketutils.hpp g++ $(CFLAGS) -o testConnection testConnection.cpp

# target for test client executable
#testClient: testClient.cpp socketutils.hpp g++ $(CFLAGS) -o testClient testClient.cpp


