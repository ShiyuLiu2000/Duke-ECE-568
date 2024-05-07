
#ifndef SERVER_HPP
#define SERVER_HPP


#include <vector>
#include <utility>
#include <algorithm>
#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <fstream>


using namespace std;
int createClient(const char* hostname, const char* port);

#endif