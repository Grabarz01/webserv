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
  unsigned int responseStatus;
	long conLen;
  void parseRequest();

  //methods
  void getReq(void);
  void postReq(void);
  void deleteReq(void);
  bool availabilityCheck(void);
  void getLocationConfig(void);
  
 public:
  RequestHandler(std::string rawRequest);
  ~RequestHandler();

  void handleRequest();
  void printRequest();

  const std::string& getRawRequest() const;
  const std::string& getResponseContent() const;
  const unsigned int getResponseStatus() const;
};

#endif