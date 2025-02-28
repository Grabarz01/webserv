#ifndef REQUESTHANDLER_HPP
#define REQUESTHANDLER_HPP

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include "Config.hpp"

class RequestHandler {
 private:
  std::string rawRequest;
  std::string method;
  std::string path;
  std::string version;
  std::map<std::string, std::string> headers;
  std::string body;
  std::string responseContent;
  std::string hostport;
  unsigned int responseStatus;
  long conLen;
  void parseRequest();

  // methods
  void getReq(void);
  void getCgiHandler(size_t pos_py, size_t pos_query);
  void postReq(void);
  void deleteReq(void);
  bool availabilityCheck(void);
  ConfigTypes::RouteConfig getLocationConfig(
      ConfigTypes::ServerConfig& server_conf);

 public:
  RequestHandler(std::string rawRequest);
  ~RequestHandler();

  void handleRequest(Config& conf);
  void printRequest();

  const std::string& getRawRequest() const;
  const std::string& getResponseContent() const;
  const unsigned int getResponseStatus() const;
};

#endif