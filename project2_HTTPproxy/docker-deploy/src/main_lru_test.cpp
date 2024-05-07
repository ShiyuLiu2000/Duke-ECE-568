#include <string>
#include "LRUCache.hpp"
int main (void) {
    LRUCache <std::string, Response> l;
    Header h;
    Response res;
    res.setResponse("HTTP/1.1 200 OK\r\nDate: Tue, 15 Feb 2022 12:30:45 GMT\r\nServer: Apache/2.4.41 (Unix)\r\nContent-Type: text/html; charset=utf-8\r\nContent-Length: 1234\r\nConnection: keep-alive\r\nCache-Control: max-age=3600, public\r\nETag: 'abc123' \r\nLast-Modified: Mon, 14 Feb 2022 18:30:00 GMT\r\nExpires: Tue, 15 Feb 2022 13:30:45 GMT\r\n");
    res.parseStatusLine("HTTP/1.1 200 OK\r\n", 15);
    res.parseHeader("HTTP/1.1 200 OK\r\nDate: Tue, 15 Feb 2022 12:30:45 GMT\r\nServer: Apache/2.4.41 (Unix)\r\nContent-Type: text/html; charset=utf-8\r\nContent-Length: 1234\r\nConnection: keep-alive\r\nCache-Control: max-age=3600, public\r\nETag: 'abc123' \r\nLast-Modified: Mon, 14 Feb 2022 18:30:00 GMT\r\nExpires: Tue, 15 Feb 2022 13:30:45 GMT\r\n", 310);
    h = res.getHeader();
    for (int i = 0; i < 10; i++) {
        std::string k = std::to_string(i);
        l.put(k, res);
    }
    std::cout << "After first put, let's see what's in it: " << std::endl;
    //l.display();
    l.displayOrder();
     for (int i = 10; i < 16; i++) {
        std::string k = std::to_string(i);
        Response resp;
        l.put(k, resp);
    }
    std::cout << "After second put, let's see what's in it: " << std::endl;
    //l.display();
    l.displayOrder();
    for (int i = 0; i < 16; i++) {
        //std::cout << "key: " << l.get(std::to_string(i)) << std::endl;
    }
    return 0;
}