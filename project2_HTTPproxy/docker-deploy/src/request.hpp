#ifndef __REQUEST__
#define __REQUEST__
#include <stdio.h>
#include <string.h>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>
#include <array>
constexpr std::size_t HOST_LENGTH = 6;


class Request {
 public:
  std::string input;
  std::string port;
  std::string host;
  std::string method;
  std::string send;
  std::string firstLine; // line typically contains the HTTP method, the URI, and the HTTP version
  std::string logInfo;


  Request(std::string request) : input(request) {
    try {
      parseHostAndPort();
      parseMethod();
      parseLine();
    } catch (const std::exception & e) {
      std::cerr << e.what() << std::endl;
      return;
    } 
  }
  bool hasValidMethod() const;
  void parseLine();
  void parseHostAndPort();
  void parseMethod();
};
#endif

