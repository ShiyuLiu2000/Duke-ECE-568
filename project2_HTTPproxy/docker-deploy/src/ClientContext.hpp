#include <pthread.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>

class ClientContext {
 private:
  int id;
  int fd;
  std::string ip;

 public:
    ClientContext() : id(), fd(), ip() {}
    void setFd(const int & myFd) {fd = myFd; }
    void setIP(const std::string & myIp) { ip = myIp; }
    void setID(const int & myId) { id = myId; }
    void setFields(const int & myFd, const std::string & myIp, const int & myId) {
        setFd(myFd);
        setIP(myIp);
        setID(myId);
    }
    int getFd() const { return fd; }
    std::string getIP() const { return ip; }
    int getID() const { return id; }
};