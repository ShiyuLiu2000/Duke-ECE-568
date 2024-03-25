#include "request.hpp"
#include "response.hpp"
#include <fstream>


int main(void) {
    
    //Request r ("GET /example/path HTTP/1.1\r\nHost: www.example.com\r\nUser-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:98.0) Gecko/20100101 Firefox/98.0\r\nAccept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8\r\nAccept-Language: en-US,en;q=0.5\r\nAccept-Encoding: gzip, deflate\r\nConnection: keep-alive\r\nReferer: https://www.example.com/previous-page\r\nCookie: sessionId=abcd1234; username=johndoe\r\n");
    /*std::cout << "host: " << r.host << std::endl;
    std::cout << "port: " << r.port<< std::endl;
    std::cout << "first_line: " << r.firstLine << std::endl;
    std::cout << "input: " << r.input << std::endl;
    std::cout << "method: " << r.method << std::endl;
    */
    
    Header h;
    Response res;
    /*const char* httpResponse = R"(
HTTP/1.1 200 OK\r\n
Date: Tue, 15 Feb 2022 12:30:45 GMT\r\n
Server: Apache/2.4.41 (Unix)\r\n
Content-Type: text/html; charset=utf-8\r\n
Content-Length: 1234\r\n
Connection: keep-alive\r\n 
Cache-Control: max-age=3600, public\r\n
ETag: "abc123"\r\n
Last-Modified: Mon, 14 Feb 2022 18:30:00 GMT\r\n
Expires: Tue, 15 Feb 2022 13:30:45 GMT\r\n
Set-Cookie: sessionID=xyz123; Path=/; Secure; HttpOnly\r\n
Strict-Transport-Security: max-age=31536000; includeSubDomains; preload\r\n
Content-Encoding: gzip\r\n
Vary: Accept-Encoding\r\n
X-Frame-Options: SAMEORIGIN\r\n
X-Content-Type-Options: nosniff\r\n
X-XSS-Protection: 1; mode=block\r\n
X-Powered-By: Express\r\n
)";

    res.setResponse(httpResponse);
    res.parseHeader(httpResponse, strlen(httpResponse));*/
    res.setResponse("HTTP/1.1 200 OK\r\nDate: Tue, 15 Feb 2022 12:30:45 GMT\r\nServer: Apache/2.4.41 (Unix)\r\nContent-Type: text/html; charset=utf-8\r\nContent-Length: 1234\r\nConnection: keep-alive\r\nCache-Control: max-age=3600, public\r\nETag: 'abc123' \r\nLast-Modified: Mon, 14 Feb 2022 18:30:00 GMT\r\nExpires: Tue, 15 Feb 2022 13:30:45 GMT\r\n");
    res.parseStatusLine("HTTP/1.1 200 OK\r\n", 15);
    res.parseHeader("HTTP/1.1 200 OK\r\nDate: Tue, 15 Feb 2022 12:30:45 GMT\r\nServer: Apache/2.4.41 (Unix)\r\nContent-Type: text/html; charset=utf-8\r\nContent-Length: 1234\r\nConnection: keep-alive\r\nCache-Control: max-age=3600, public\r\nETag: 'abc123' \r\nLast-Modified: Mon, 14 Feb 2022 18:30:00 GMT\r\nExpires: Tue, 15 Feb 2022 13:30:45 GMT\r\n", 310);
    h = res.getHeader();
    std::cout << "contentLength " << h.contentLength << std::endl;
    std::cout << "contentType " << h.contentType << std::endl;
    std::cout << "etag " << h.etag << std::endl;
    std::cout << "expirationStr " << h.expirationStr << std::endl;
    std::cout << "maxAge " << h.maxAge << std::endl;
    std::cout << "no_cache " << h.no_cache << std::endl;
    
    h.printHeader(); 
    std::cout << "Expire Parse Timer: " << std::endl;
    h.expirationTime.display();
    std::cout << "Response Parse Timer: " << std::endl;
    h.responseTime.display();
    return EXIT_SUCCESS;
}