#include <vector>
#include "request.hpp"
#include "response.hpp"
#include "proxy.hpp"
#include "ClientContext.hpp"
#include "socketutils.hpp"
 
//volatile sig_atomic_t Proxy::shutdownFlag = 0;
LRUCache<std::string, Response> Proxy::cache;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
std::ofstream logFile("/var/log/erss/proxy.log");
const char * DEFAULT_PORT;
void Proxy::run() {
    int serverSocket = startServer(portNum);
    if (serverSocket < 0) {
        pthread_mutex_lock(&mutex);
        logFile << "Failed to set server up to listen for incoming requests";
        pthread_mutex_unlock(&mutex);
    }
    int id = 0;
    while (true /*&& !shutdownFlag*/) {
        int client_connection_fd;
        std::string ip;
        client_connection_fd = acceptClientConnection(serverSocket, ip);
        if (client_connection_fd < 0) {
            pthread_mutex_lock(&mutex);
            logFile << "ERROR: Failed to connect to client" << std::endl;
            pthread_mutex_unlock(&mutex);
        }
        pthread_t thread;
        pthread_mutex_lock(&mutex);
        ClientContext * clientcontext = new ClientContext();
        clientcontext->setFields(client_connection_fd, ip, id);
        id++;
        pthread_mutex_unlock(&mutex);
        pthread_create(&thread, NULL, handleRequest, clientcontext);
        // Check shutdownFlag and break if set
        /*if (shutdownFlag)
            cleanUpResources(serverSocket, client_connection_fd,requestObject);
            std::cout << "Shutting down gracefully. Cleanup complete." << std::endl;
            break;
            */
    }
    
}

void * Proxy::handleRequest(void * ptr) {
 
    // Retrieve client context from the pointer
    ClientContext * clientcontext = (ClientContext *)ptr;
    int client_connection_fd = clientcontext->getFd();

    char request[BUFFER_SIZE] = {0};
    // Receive the request from the client
    int length = recv(client_connection_fd, request, sizeof(request), 0);
    if (length <= 0) {
        // Log an error for a bad request
        pthread_mutex_lock(&mutex);
        logFile << clientcontext->getID() << ": Bad request: length is <= 0" << std::endl;
        pthread_mutex_unlock(&mutex);
    }
    // Convert the received request to a std::string
    std::string req = std::string(request, length);
    if (isEmptyInput(req)) {
        // If the request is empty, return nullptr
        return nullptr;
    }
    // Parse the request
    Request * requestObject = new Request(req); 
    if (!requestObject->hasValidMethod()) {
        // Log an error for an invalid method
        const char * badRequest = "HTTP/1.1 400 Bad Request";
        pthread_mutex_lock(&mutex);
        logFile << clientcontext->getID() << ": " << badRequest << " Invalid Method: " << requestObject->method << std::endl;
        pthread_mutex_unlock(&mutex);
        return nullptr;
    }
    // Log the request information
    pthread_mutex_lock(&mutex);
    logFile << clientcontext->getID() << ": \"" << requestObject->firstLine << "\" from "
          << clientcontext->getIP() << " @ " << getCurrentUTCTime() + "\0";
    pthread_mutex_unlock(&mutex);

    // Extract host and port from the request
    const char * host = requestObject->host.c_str();
    const char * port = requestObject->port.c_str();
    DEFAULT_PORT = port;
    // Connect to the server
    int server_fd = buildClient(host, port);
    if (server_fd < 0) {
        // Log an error if failed to connect to the server
        std::cerr << "Failed to build client" << std::endl;
        return nullptr;
    }

    if (requestObject->method == "CONNECT") {
        // Handle CONNECT request
        pthread_mutex_lock(&mutex);
        logFile << clientcontext->getID() << ": " << "Requesting \"" << requestObject->firstLine << "\" from " << host << std::endl;
        pthread_mutex_unlock(&mutex);
        handleConnectRequest(client_connection_fd, server_fd, clientcontext->getID());
        pthread_mutex_lock(&mutex);
        logFile << clientcontext->getID() << ": Connection has been closed" << std::endl;
        pthread_mutex_unlock(&mutex);
    }
    else if (requestObject->method == "POST") {
        // Handle POST request
        pthread_mutex_lock(&mutex);
        logFile << clientcontext->getID() << ": " << "Requesting \"" << requestObject->firstLine << "\" from " << host << std::endl;
        pthread_mutex_unlock(&mutex);
        handlePOSTRequest(client_connection_fd, server_fd, request, length, clientcontext->getID(), host);
    }
    else {
        // Handle GET request
        assert(requestObject->method == "GET");
        int id = clientcontext->getID();
        bool isValid = false;
        std::string line = requestObject->firstLine;
        pthread_mutex_lock(&mutex);
        Response result = cache.get(line);
        pthread_mutex_unlock(&mutex);
        if (result.isEmpty()) {
            // If not found in cache, request from the server
            pthread_mutex_lock(&mutex);
            logFile << id << ": failed to locate " << clientcontext->getID() << " in cache" << std::endl;
            pthread_mutex_unlock(&mutex);
            pthread_mutex_lock(&mutex);
            logFile << clientcontext->getID() << ": "
                << "Requesting \"" << requestObject->firstLine << "\" from " << host << std::endl;
            pthread_mutex_unlock(&mutex);
            send(server_fd, request, length, 0);
            handleGetRequest(client_connection_fd, server_fd, clientcontext->getID(), host, requestObject->firstLine);
        }
        else { 
            // Found in cache
            if (result.getHeader().no_cache) {
                // If no-cache, check Etag and Last Modified
                if (shouldUseCache(result, requestObject->input, server_fd, id) == false) {
                    // If not usable, forward the request to the host
                    
                    forwardRequestToServer(id, requestObject->firstLine, request, length, client_connection_fd, server_fd, host);
                }
                else {
                    // Send the cached response to the client
                    sendCachedResponseToClient(result, id, client_connection_fd);
                }
            }
            else {
                // Handle cached response
                isValid = handleCachedResponse(server_fd, *requestObject, requestObject->firstLine, result, clientcontext->getID());
                if (!isValid) {
                    // If not valid, forward the request to the server
                    forwardRequestToServer(id, requestObject->firstLine, request, length, client_connection_fd, server_fd, host);
                }
                else {
                    // Send from cache
                    sendCachedResponseToClient(result, id, client_connection_fd);
                }
            }
            // Display the cache
            pthread_mutex_lock(&mutex);
            cache.display();
            pthread_mutex_unlock(&mutex);
        }
    }
    // Close connections
    close(server_fd);
    close(client_connection_fd);
    return nullptr;
}

void Proxy::updateRequestForRedirection(std::string & firstLine, const std::string& newLocation) {
    // firstLine format=> "GET /path/to/resource HTTP/1.1"
    // Extract the path from the original request
    std::size_t pathStart = firstLine.find(' ');
    std::size_t pathEnd = firstLine.rfind(' ');

    if (pathStart != std::string::npos && pathEnd != std::string::npos && pathStart < pathEnd) {
        // Extract the path part of the original request
        std::string path = firstLine.substr(pathStart + 1, pathEnd - pathStart - 1);

        // Update the first line with the new location
        firstLine = "GET " + newLocation + path + " HTTP/1.1";
    }
}

void Proxy::checkRedirection(Response & response, std::string & firstLine, int serverFd, int id) {
    if (response.getHeader().statusCode == "302") {
        while (true) {
            // Extract the new location from the "Location" header
            std::string newLocation = response.getHeader().location;
            pthread_mutex_lock(&mutex);
            logFile << id << ": Redirected to " << newLocation << std::endl;
            pthread_mutex_unlock(&mutex);
            // Update the request to the new location
            updateRequestForRedirection(firstLine, newLocation);

            // Close the current connection to the server
            close(serverFd);

            // Build a new connection to the redirected location
            serverFd = buildClient(newLocation.c_str(), DEFAULT_PORT);
        }
    }
}
bool Proxy::shouldUseCache(Response& response, const std::string& input, int serverFd, int requestId) {
  // If ETag and LastModified are not present, use the cache
  if (response.getHeader().etag.empty() && response.getHeader().lastModified.empty()) {
    return true;
  }

  // Prepare the modified input for revalidation
  std::string modifiedInput = input;
  if (!response.getHeader().etag.empty()) {
    std::string etagHeader = "If-None-Match: " + response.getHeader().etag + "\r\n";
    modifiedInput.insert(modifiedInput.length() - 2, etagHeader);
  }
  if (!response.getHeader().lastModified.empty()) {
    std::string lastModifiedHeader = "If-Modified-Since: " + response.getHeader().lastModified + "\r\n";
    modifiedInput.insert(modifiedInput.length() - 2, lastModifiedHeader);
  }

  // Send the modified request to the server for revalidation
  if (send(serverFd, modifiedInput.c_str(), modifiedInput.size(), 0) > 0) {
    std::cout << "Revalidation: Request sent successfully!\n";
  }

  // Receive the response from the server
  char newResponse[BUFFER_SIZE] = {0};
  int newLength = recv(serverFd, newResponse, sizeof(newResponse), 0);
  if (newLength <= 0) {
    std::cout << "[Revalidation] Failed to receive from the server in checktime" << std::endl;
  }

  // Check if the response indicates a successful revalidation
  std::string checkNew(newResponse, newLength);
  if (checkNew.find("HTTP/1.1 200 OK") != std::string::npos) {
    pthread_mutex_lock(&mutex);
    logFile << requestId << ": in cache, requires validation" << std::endl;
    pthread_mutex_unlock(&mutex);
    return false;  // Do not use the cache, revalidation is required
  }

  return true;  // Use the cache
}

void Proxy::sendCachedResponseToClient(Response & res, int id, int client_fd) {
    //char cachedResponse [res.getResponseSize()];
    //strcpy(cachedResponse, res.getResponse());
    std::string cachedResponse = res.getResponse();
    send(client_fd, cachedResponse.c_str(), res.getResponseSize(), 0);
    pthread_mutex_lock(&mutex);
    //logFile << id << ": Responding: \"" << res.getStatusLine() << "from cache \"" << std::endl;
    logFile << id << ": in cache, valid" << std::endl;
    pthread_mutex_unlock(&mutex);
}

void Proxy::forwardRequestToServer(int id, const std::string & line, char * request, int length, int client_fd, int server_fd, const char * host) {
    pthread_mutex_lock(&mutex);
    logFile << id << ": "
            << "Requesting \"" << line << "\" from " << host << std::endl;
    pthread_mutex_unlock(&mutex);
    send(server_fd, request, length, 0);
    std::cout << "FORWARDING TO THE HOST: " << host << std::endl;
    handleGetRequest(client_fd, server_fd, id, host, line);
}

std::string Proxy::receiveWithContentLength(int sendFd, const char * serverMessage, int messageLength, int contentLength){
    std::vector<char> buffer(BUFFER_SIZE);

    std::string receivedContent;
    assert(contentLength >= 0);
    while (receivedContent.size() < (size_t) contentLength) {
        int bytesReceived = recv(sendFd, buffer.data(), BUFFER_SIZE, 0);

        if (bytesReceived <= 0) {
            if (bytesReceived == 0) {
                // Connection closed by the other end
                break;
            } else {
                // Error during recv
                throw std::runtime_error("Error receiving data from the socket");
            }
        }
        // Append the received data to the string
        receivedContent.append(buffer.data(), bytesReceived);
    }
    return receivedContent;
}

void Proxy::handlePOSTRequest(int clientFd,
                      int serverFd,
                      const char* requestMessage,
                      int messageLength,
                      int requestId,
                      const char* host) {
    
    int contentLength = getLength(requestMessage, messageLength); // Get the length of the client request

  if (contentLength != -1) {
    // Forward the entire client request to the server
    std::string request;
    try {
    request = receiveWithContentLength(clientFd, requestMessage, messageLength, contentLength);
    } catch (const std::exception & e) {
        std::cerr << e.what() << std::endl;
        return;
    }
    //char toSend[request.length() + 1];
    //strcpy(toSend, request.c_str());
   //send(serverFd, toSend, sizeof(toSend), MSG_NOSIGNAL);
    //send(serverFd, request.c_str(), request.length(), MSG_NOSIGNAL);

    // Receive the response from the server
    char response[BUFFER_SIZE] = {0};
    int responseLength = recv(serverFd, response, sizeof(response), MSG_WAITALL);

    if (responseLength != 0) {
      // Parse the request line for logging
      Response parsedResponse;
      try {
        parsedResponse.parseStatusLine(requestMessage, messageLength);
      } catch (const std::exception & e) {
        std::cerr << e.what() << " failed to parse status line" << std::endl;
        return;
      }
      
      // Log the received response
      pthread_mutex_lock(&mutex);
      logFile << requestId << ": Received \"" << parsedResponse.getStatusLine() << "\" from " << host << std::endl;
      pthread_mutex_unlock(&mutex);
    
      // Send the received response back to the client
      send(clientFd, response, responseLength, MSG_NOSIGNAL);

      // Log the response in the logFile
      pthread_mutex_lock(&mutex);
      logFile << requestId << ": Responding \"" << parsedResponse.getStatusLine() << "\"" << std::endl;
      pthread_mutex_unlock(&mutex);
    } else {
      std::cout << "Server socket closed!" << std::endl;
    }
  }
}

void Proxy::handleGetRequest(int clientFd,
                        int serverFd,
                        int id,
                        const char * host,
                        std::string statusLine) {
    char serverMessage[BUFFER_SIZE] = {0};
    
    int messageLength = recv(serverFd, serverMessage, BUFFER_SIZE, 0);
    std::string temp(serverMessage, 512);
    if (messageLength == 0) {
        return;
    }
    else {
        pthread_mutex_lock(&mutex);
        logFile << id << ": Received response from " << host << std::endl;
        pthread_mutex_unlock(&mutex);
    }
    
    Response resp; 
    resp.setResponse(temp);
    resp.parseStatusLine(serverMessage, messageLength);
    resp.parseHeader(serverMessage, messageLength);
    resp.getHeader().printHeader();
    /*if (resp.getHeader().statusCode == "302") {
        while (true) {
            // Extract the new location from the "Location" header
            std::string newLocation = resp.getHeader().location;
            pthread_mutex_lock(&mutex);
            logFile << id << ": Redirected to " << newLocation << std::endl;
            pthread_mutex_unlock(&mutex);
            // Update the request to the new location
            updateRequestForRedirection(statusLine, newLocation);
            
            // Close the current connection to the server
            close(serverFd);

            // Build a new connection to the redirected location
            serverFd = buildClient(newLocation.c_str(), DEFAULT_PORT);
            send(server_fd, request, length, 0);
        }*/

    pthread_mutex_lock(&mutex);
    logFile << id << ": Received \"" << resp.getStatusLine() << "\" from " << host << std::endl;
    pthread_mutex_unlock(&mutex);
    
    bool isChunk = containsChunkedTransferEncoding(serverMessage, messageLength);

    if (isChunk) {
        pthread_mutex_lock(&mutex);
        logFile << id << ": not cacheable because it is chunked" << std::endl;
        pthread_mutex_unlock(&mutex);

        send(clientFd, serverMessage, messageLength, 0);
        char truncatedMessage[BUFFER_SIZE] = {0};
        
        while (true) {
            int len = recv(serverFd, truncatedMessage, BUFFER_SIZE, 0);
            if (len <= 0) {
                std::cout << "chunked, break" << std::endl;
                break;
            }
            send(clientFd, truncatedMessage, len, 0);
        }
    } else {
        bool no_store = false;
        std::string serverMessageStr(serverMessage, messageLength);
        std::size_t nostorePosition;
        
        if ((nostorePosition = serverMessageStr.find("no-store")) != std::string::npos) {
            no_store = true;
        }

        resp.parseHeader(serverMessage, messageLength);
        logResponseNotes(resp, id);
        int contentLength = getLength(serverMessage, messageLength);

        if (contentLength >= 0) {
            std::string msg = receiveWithContentLength(serverFd, serverMessage, messageLength, contentLength);
            resp.setResponse(msg);

            if (msg.length() >= NORMAL_MESSAGE_SIZE_LIMIT) {
                std::vector<char> largeMessage(msg.begin(), msg.end());
                const char * messageToSend = largeMessage.data();
                send(clientFd, messageToSend, msg.length(), 0);
            } else {
                std::string sendResponse = msg;
                resp.setResponse(msg);
                send(clientFd, sendResponse.c_str(), sendResponse.length(), 0);
            }
        } else {
            std::string serverMessageStr(serverMessage, messageLength);
            resp.setResponse(serverMessageStr);
            send(clientFd, serverMessage, messageLength, 0);
        }
        printCacheLog(resp, no_store, statusLine, id);
    }

    std::string logResponse(serverMessage, messageLength);
    size_t pos = logResponse.find_first_of("\r\n");
    std::string toLog = logResponse.substr(0, pos);

    
    pthread_mutex_lock(&mutex);
    logFile << id << ": Responding \"" << toLog << "\"" << std::endl;
    pthread_mutex_unlock(&mutex);
}

void Proxy::handleConnectRequest(int clientFd, int serverFd, int id) {
    const char * ok_response = "HTTP/1.1 200 OK\r\n\r\n";
    send(clientFd, ok_response, strlen(ok_response), 0);

    pthread_mutex_lock(&mutex);
    logFile << id << ": Responding \"" << ok_response << "\"" << std::endl;
    pthread_mutex_unlock(&mutex);

    fd_set readfds;
    int max_fd = std::max(serverFd, clientFd) + 1;

    while (true) {
        FD_ZERO(&readfds);
        FD_SET(serverFd, &readfds);
        FD_SET(clientFd, &readfds);

        select(max_fd, &readfds, NULL, NULL, NULL);

        for (int fd : {serverFd, clientFd}) {
            if (FD_ISSET(fd, &readfds)) {
                char message[BUFFER_SIZE] = {0};
                int len = recv(fd, message, sizeof(message), 0);

                if (len <= 0) {
                    return;
                } else {
                    int otherFd = (fd == serverFd) ? clientFd : serverFd;

                    if (send(otherFd, message, len, 0) <= 0) {
                        return;
                    }
                }
            }
        }
    }
}

int Proxy::getLength(const char* serverMessage, int messageLength) {
    std::string msg(serverMessage, messageLength);

    size_t pos = msg.find("Content-Length: ");
    if (pos != std::string::npos) {
        std::size_t endOfHeading = msg.find("\r\n\r\n");

        int partialBodyLength = messageLength - static_cast<int>(endOfHeading) - 4;

        std::size_t end = msg.find("\r\n", pos);
        std::string contentLengthStr = msg.substr(pos + 16, end - pos - 16);

        try {
            int contentLength = std::stoi(contentLengthStr);
            return contentLength - partialBodyLength;
        } catch (const std::invalid_argument& e) {
           pthread_mutex_lock(&mutex);
            logFile << ": Content-Length is not in a valid numerical format: \"" << e.what() << std::endl;
            pthread_mutex_unlock(&mutex);
            return -1;
        }
    }

    return -1;  // Content-Length not found
}

bool Proxy::containsChunkedTransferEncoding(const char* server_msg, int mes_len) {
    // Convert the server_msg to a string
    std::string msg(server_msg, mes_len);

    // Check if "chunked" is found in the message
    return (msg.find("chunked") != std::string::npos);
}

/*
    The function getTime returns a std::string representation of the current time in Coordinated Universal Time (UTC). It uses the C Standard Library functions time to get the current time in seconds since the epoch and gmtime to convert it to the broken-down time structure in UTC. Finally, asctime is used to convert the structure back into a string representation, which is then returned as a std::string.

*/
std::string Proxy::getCurrentUTCTime() {
  // Get the current time in seconds since the epoch
  time_t currentTimeInSeconds = time(0);

  // Convert the current time to the broken-down time structure in UTC
  struct tm * utcTimeStructure = gmtime(&currentTimeInSeconds);

  // Convert the UTC time structure to a string representation
  const char * timeString = asctime(utcTimeStructure);

  // Return the UTC time as a std::string
  return std::string(timeString);
}

void Proxy::check502Response(const std::string& entireMessage, int clientFd, int requestId) {
  // Check if the provided message contains the end of HTTP headers
  if (entireMessage.find("\r\n\r\n") == std::string::npos) {
    // Prepare the "502 Bad Gateway" response
    const char *badGatewayResponse = "HTTP/1.1 502 Bad Gateway";

    // Send the "502 Bad Gateway" response to the client
    send(clientFd, badGatewayResponse, strlen(badGatewayResponse), 0);

    // Log the response in the logFile
    pthread_mutex_lock(&mutex);
    logFile << requestId << ": Responding \"" << badGatewayResponse << "\"" << std::endl;
    pthread_mutex_unlock(&mutex);
  }
}

void Proxy::printCacheLog(const Response & parsedResponse, bool noStore, const std::string & requestLine, int requestId) {

  if (parsedResponse.getStatusLine().find("HTTP/1.1 200 OK") != std::string::npos) {
    if (noStore) {
      pthread_mutex_lock(&mutex);
      logFile << requestId << ": not cacheable because NO STORE" << std::endl;
      pthread_mutex_unlock(&mutex); 
      return;
    }

    if (parsedResponse.getHeader().maxAge != -1) {
      time_t expirationTime = mktime(parsedResponse.getHeader().responseTime.getTimeStruct()) + parsedResponse.getHeader().maxAge;
      struct tm* expirationStruct = gmtime(&expirationTime);
      const char* expirationTimeString = asctime(expirationStruct);
      pthread_mutex_lock(&mutex);
      logFile << requestId << ": cached, expires at " << expirationTimeString << std::endl;
      pthread_mutex_unlock(&mutex);
    }
    else if (!parsedResponse.getHeader().expirationStr.empty()) {
      pthread_mutex_lock(&mutex);
      logFile << requestId << ": cached, expires at " << parsedResponse.getHeader().expirationStr << std::endl;
      pthread_mutex_unlock(&mutex);
    }
    Response storedResponse(parsedResponse);
    pthread_mutex_lock(&mutex);
    cache.put(requestLine, storedResponse);
    pthread_mutex_unlock(&mutex);
    
  }
}

bool Proxy::handleCachedResponse(int serverFd,
                                 Request& requestParser,
                                 const std::string& requestLine,
                                 Response& response,
                                 int id) {
    if (response.getHeader().maxAge != -1) {
        time_t currentTime = std::time(0);
        time_t responseTime = std::mktime(response.getHeader().responseTime.getTimeStruct());
        int maxAge = response.getHeader().maxAge;

        if (responseTime + maxAge <= currentTime) {
            pthread_mutex_lock(&mutex);
            cache.remove(requestLine);
            pthread_mutex_unlock(&mutex);
            time_t expirationTime = responseTime + maxAge;
            struct tm* expirationStruct = std::gmtime(&expirationTime);
            const char* expirationString = std::asctime(expirationStruct);

            pthread_mutex_lock(&mutex);
            logFile << id << ": in cache, but expired at " << expirationString << std::endl;
            pthread_mutex_unlock(&mutex);
            return false;
        }
    }

    if (!response.getHeader().expirationStr.empty()) {
        time_t currentTime = std::time(0);
        time_t expireTime = std::mktime(response.getHeader().expirationTime.getTimeStruct());

        if (currentTime > expireTime) {
            pthread_mutex_lock(&mutex);
            cache.remove(requestLine);
            pthread_mutex_unlock(&mutex);
            time_t expirationTime = std::mktime(response.getHeader().expirationTime.getTimeStruct());
            struct tm* expirationStruct = std::gmtime(&expirationTime);
            const char* expirationString = std::asctime(expirationStruct);

            pthread_mutex_lock(&mutex);
            logFile << id << ": Cached response expired at " << expirationString << std::endl;
             pthread_mutex_unlock(&mutex);
            return false;
        }
    }

    if (!shouldUseCache(response, requestParser.input, serverFd, id)) {
        return false;
    }
    pthread_mutex_lock(&mutex);
    logFile << id << ": in cache, valid" << std::endl;
    pthread_mutex_lock(&mutex);
    return true;
}

void Proxy::logResponseNotes(const Response& parseRes, int requestId) {

  if (parseRes.getHeader().maxAge != -1) {
    pthread_mutex_lock(&mutex);
    logFile << requestId << ": NOTE Cache-Control: max-age=" << parseRes.getHeader().maxAge << std::endl;
    pthread_mutex_unlock(&mutex);
  }

  if (!parseRes.getHeader().expirationStr.empty()) {
    pthread_mutex_lock(&mutex);
    logFile << requestId << ": NOTE Expires: " << parseRes.getHeader().expirationStr << std::endl;
    pthread_mutex_unlock(&mutex);
  }

  if (parseRes.getHeader().no_cache) {
    pthread_mutex_lock(&mutex);
    logFile << requestId << ": NOTE Cache-Control: no-cache" << std::endl;
    pthread_mutex_unlock(&mutex);
  }

  if (!parseRes.getHeader().etag.empty()) {
    pthread_mutex_lock(&mutex);
    logFile << requestId << ": NOTE ETag: " << parseRes.getHeader().etag << std::endl;
    pthread_mutex_unlock(&mutex);
  }

  if (!parseRes.getHeader().lastModified.empty()) {
    pthread_mutex_lock(&mutex);
    logFile << requestId << ": NOTE Last-Modified: " << parseRes.getHeader().lastModified << std::endl;
    pthread_mutex_unlock(&mutex);
  }
}

/*void Proxy::handleShutdown(int signal)
{
    // Signal the shutdown by setting the flag
    shutdownFlag = 1;
    std::cout << "Received signal " << signal << ". Shutting down..." << std::endl;
}

void Proxy::cleanUpResources(int serverSocket, int client_connection_fd, Request * requestObject) {
close(serverSocket);
close(client_connection_fd);
delete requestObject;
pthread_mutex_unlock(&mutex);
logFile.close();
}*/