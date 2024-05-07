#ifndef PROXY_H
#define PROXY_H
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <cassert>
#include "request.hpp"
#include "pthread.h"
#include "response.hpp"
#include "LRUCache.hpp"
//#include <csignal>


extern pthread_mutex_t mutex;
extern std::ofstream logFile;
constexpr std::size_t BUFFER_SIZE = 65536;
constexpr std::size_t NORMAL_MESSAGE_SIZE_LIMIT = 10000000;
class Proxy {
 private:
  const char * portNum;
  static LRUCache <std::string, Response> cache;
  //static volatile sig_atomic_t shutdownFlag;

 public:
    Proxy(const char * proxyPort) : portNum(proxyPort)/*, cache()*/ { 
      /*static void handleShutdown(int signal);*/
      //signal(SIGINT, handleShutdown);
      //signal(SIGTERM, handleShutdown);
      }
    void run();
    static void * handleRequest(void * arg);
    static void handleConnectRequest(int clientFd, int serverFd, int id);
    static void handleGetRequest(int clientFd,
                        int serverFd,
                        int id,
                        const char * host,
                        std::string statusLine);
  static void handlePOSTRequest(int clientFd,
                         int serverFd,
                         const char * requestMessage,
                         int length,
                         int id,
                         const char * host);
  static std::string receiveWithContentLength(int sendFd,
                                    const char * serverMessage,
                                    int messageLength,
                                    int contentLength);
  static int getLength(const char * serverMessage, int messageLength);
  static bool containsChunkedTransferEncoding(const char * server_msg, int mes_len);
  static std::string getCurrentUTCTime();
  static bool handleCachedResponse(int serverFd,
                                 Request& requestParser,
                                 const std::string& requestLine,
                                 Response& response,
                                 int id);
  static void printCacheLog(const Response& parsedResponse, bool noStore, const std::string & requestLine, int requestId);
  //void forwardRequestToServer(int client_fd, const char* host, const std::string& request);

  static void forwardRequestToServer(int id,
                         const std::string & line,
                         char * request,
                         int length,
                         int client_fd,
                         int server_fd,
                         const char * host);
  static void sendCachedResponseToClient(Response & res, int id, int client_fd);
  static bool shouldUseCache(Response & rep, const std::string & input, int server_fd, int id);
  static void check502Response(const std::string& entireMessage, int clientFd, int requestId);
  //static void handleShutdown(int signal);
  //static void cleanUpResources(int serverSocket, int client_connection_fd, Request * requestObject);
  static void checkRedirection(Response & response, std::string & firstLine, int serverFd, int id);
  static void updateRequestForRedirection(std::string & firstLine, const std::string& newLocation);
  static void logResponseNotes(const Response& parseRes, int requestId);
};
#endif



