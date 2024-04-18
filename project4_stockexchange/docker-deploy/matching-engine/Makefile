CC=g++
CFLAGS=-O3
EXTRAFLGS=-lpqxx -lpq -pthread

all: server #test_local

# test_local: database.hpp tinyxml2.h
# 	$(CC) $(CFLAGS) -o test_local database.cpp tinyxml2.cpp $(EXTRAFLGS)

server: main.cpp server.hpp database.hpp tinyxml2.h
	$(CC) $(CFLAGS) -o server main.cpp server.cpp database.cpp tinyxml2.cpp $(EXTRAFLGS)

.PHONY: clean

clean:
	rm -f test_local server *.o *~