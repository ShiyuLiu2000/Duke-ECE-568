#ifndef MAIN_HPP
#define MAIN_HPP

#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <mutex>
#include <netinet/in.h>
#include <sstream>
#include <string>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>
#include <map>
#include <pqxx/pqxx>
#include <thread>
#include <fstream> 
#include <set>
#include <unistd.h> // for sleep()
#include <iomanip>
#include <ctime>
#include "server.hpp"
#include "world_ups.pb.h"
#include "amazon_ups.pb.h"
#include "message.hpp"
#include "database.hpp"
using namespace pqxx;
using namespace std;
extern connection *C;
extern int worldFD;
extern mutex mutexForWorld;
extern mutex mutexForAmazon;
extern mutex mutexForTruck;
extern mutex mutexForPackage;
extern mutex mutexForUser;
extern mutex mutexForDelivery;
extern int worldID;
extern int amazonFD;




 extern std::set<int> waitingForAckSet;
extern  std::set<int> receivedSeqSet;

extern int seqnumGlobal;

 extern const char* worldHostName ;
 extern const char* worldPort;
 extern const char* amazonHostName ;
 extern const char* amazonPort;



#endif