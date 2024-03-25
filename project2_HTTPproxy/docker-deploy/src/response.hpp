#ifndef RESPONSE_H
#define RESPONSE_H
#include <iostream>
#include <string>
#include <cstdlib>
#include <ctime>
#include "time.hpp"

// Forward declaration
class TimeParser;

struct Header {
    
    std::string contentType;//indicate the type or media type of the content being sent. It specifies the format of the data, such as HTML, JSON, XML, or image formats. eg.Content-Type: text/html
    std::string contentLength;//Content-Length header specifies the size of the payload or body of the HTTP response in bytes.eg.Content-Length: 1024
    std::string server;//identify information about the server software that handled the request. It typically includes the name and version of the server software. eg.Server: Apache/2.4.41 (Unix)
    std::string etag;//see below for details. eg. "686897696a7c876b7e"
    std::string lastModified;//indicates the date and time when the resource was last modified on the server. eg. Last-Modified: Thu, 15 Feb 2024 12:30:00 GMT
    std::string expirationStr;// indicates the date and time at which the response is considered stale and should no longer be cached by the client.eg. Expires: Thu, 15 Feb 2024 12:30:00 GMT
    std::string location;
    std::string statusCode;
    TimeParser expirationTime;
    TimeParser responseTime;
    bool no_cache;//indicate that a request or response must not be served from the cache without first validating with the origin server.eg.Cache-Control: no-cache
    int maxAge;//maximum amount of time in seconds that a cached representation is considered fresh. eg. Cache-Control: max-age=3600

    Header () : contentType(""), contentLength(""), server(""), etag(""), 
    lastModified(""), expirationStr(""), location(), statusCode(), expirationTime(), responseTime(), no_cache(false), maxAge(-1) {}

    bool isEmpty () const {
        return contentType.empty() && contentLength.empty() && server.empty()
        && etag.empty() && lastModified.empty() && expirationStr.empty();
    }

/*
    When a server generates a response, it can include an ETag header to uniquely identify the current version of the resource. The client can then include this ETag in subsequent requests for the same resource. If the resource hasn't changed since the provided ETag, the server can respond with a 304 Not Modified status code, indicating that the client's cached version is still valid. If the resource has changed, the server can provide the updated content along with a new ETag.
*/
    // Helper method to print header for debugging purposes
    void printHeader() const {
        std::cout << "Content-Type: " << contentType << std::endl;;
        std::cout << "Content-Length: " << contentLength << std::endl;
        std::cout << "Server: " << server << std::endl;
        std::cout << "ETag: " << etag << std::endl;
        std::cout << "Expires: " << expirationStr << std::endl;
        //std::cout << "expiration time: " << expirationTime << std::endl;
        std::cout << "Last-Modified: " << lastModified << std::endl;
        std::cout << "maxAge: " << maxAge << std::endl;
        std::cout << "Cache-Control: " << !no_cache << std::endl;
        if (!no_cache) {
            std::cout << "max-age (cache): " << maxAge << std::endl;
        }
    }
};
typedef struct Header Header;

/*
    HTTP RESPONSE
Here's an example of an HTTP response:
plaintext

Copy code
HTTP/1.1 200 OK
Date: Tue, 15 Feb 2022 12:30:45 GMT
Server: Apache/2.4.41 (Unix)
Content-Type: text/html; charset=utf-8
Content-Length: 1234
Connection: keep-alive
Cache-Control: max-age=3600, public
ETag: "abc123"
Last-Modified: Mon, 14 Feb 2022 18:30:00 GMT
Expires: Tue, 15 Feb 2022 13:30:45 GMT
Set-Cookie: sessionID=xyz123; Path=/; Secure; HttpOnly
Strict-Transport-Security: max-age=31536000; includeSubDomains; preload
Content-Encoding: gzip
Vary: Accept-Encoding
X-Frame-Options: SAMEORIGIN
X-Content-Type-Options: nosniff
X-XSS-Protection: 1; mode=block
X-Powered-By: Express
In this example, the status line indicates a successful response with a status code of 200 (OK). 
The headers provide additional information, including the server type, date of the response, content type, and the length of the response body.
 The actual content would follow in the response body.
*/
class Response {
private:
    std::string response;
    Header header;
    std::string statusLine;
   friend std::ostream& operator<<(std::ostream& os, const Response& response);

    void parseGenericHeaderField(const std::string& hdr, const std::string& fieldName, std::string & fieldValue);
public:
    Response() { 
    }
    void setResponse(const std::string & resp);
    //void setHeaders(const Header & newHeaders);
    void parseStatusLine(const char * line, size_t length);
    void setStatusLine(const std::string & statLine) { statusLine = statLine;}
    void parseHeader(const char * header, size_t length);
    std::string extractStatusCode(const std::string & httpResponse);
    //void parseBody();
    const char * getResponse() const;
    std::string getResponseStr() const;
    int getResponseSize() const;
    std::string getStatusLine () const;
    Header getHeader() const;
    bool isEmpty() const;
    Response& operator=(const Response& other);
};
#endif