#ifndef REQUESTHANDLER_HPP
#define REQUESTHANDLER_HPP

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>

class RequestHandler {
 private:
  std::string rawRequest;
  std::string method;
  std::string path;
  std::string version;
  std::map<std::string, std::string> headers;
  std::string body;
  std::string responseContent;

  void parseRequest();

 public:
  RequestHandler(std::string rawRequest);
  ~RequestHandler();

  void printRequest();
  const std::string& RequestHandler::getRawRequest() const;
  void handleRequest();
  const std::string& getResponseContent() const; 
};

#endif