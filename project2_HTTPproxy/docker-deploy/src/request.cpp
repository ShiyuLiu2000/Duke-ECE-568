#include "request.hpp"

bool Request::hasValidMethod()const {
    return method == "POST" || method == "GET" || method == "CONNECT";
}
void Request::parseLine() {
    std::string delimeter = "\r\n"; //according to the HTTP specification, lines are terminated by a CRLF sequence ("\r\n").
    std::size_t found = input.find(delimeter);
    if (found == std::string::npos) {
        throw std::invalid_argument("Improper formatting passed for line");
        return;
    }
    firstLine = input.substr(0, found);
}

void Request::parseMethod() {
    std::size_t method_start = input.find_first_not_of(" ");
    if (method_start == std::string::npos) {
        throw std::invalid_argument("Failed to extract method from the request: there must be a non-whitespace character after method.");
    }
    std::size_t method_end = input.find(" ", method_start);
    if (method_end == std::string::npos) {
        throw std::invalid_argument("Failed to extract method from the request: there should be a whitespace after the METHOD.");
    }
    // Extract the method
    method = input.substr(method_start, method_end - method_start);
   
}
void Request::parseHostAndPort() {
    try {
        //search for the "Host: " substring, then extract the relevant information based on the presence of a colon :
        std::size_t found_host = input.find("Host: ");
        if (found_host == std::string::npos) {
            throw std::invalid_argument("Invalid argument: there is no occurrence of 'Host' in request");
        }
        std::size_t found_host_end = input.find("\r\n", found_host);
         if (found_host_end == std::string::npos) {
            throw std::invalid_argument("Invalid argument: 'Host' header is not terminated properly");
        }
        std::string host_line = input.substr(found_host + HOST_LENGTH, found_host_end - (found_host + HOST_LENGTH));
        std::size_t port_start;
        if((port_start = host_line.find(":")) != std::string::npos) {
            host = host_line.substr(0, port_start);
            //std::cout << "In request, host is: " << host << std::endl;
            port = host_line.substr(port_start + 1);
        }
        else {
            host = host_line;
            port = "80";
        }

    }
    catch (std::invalid_argument & e) {
        std::cerr << "Invalid argument supplied: " << e.what() << std::endl;
    }
    catch (std::exception & e) {
        std::cerr << "Error " << e.what() << std::endl;
    }
}

