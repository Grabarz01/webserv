#ifndef REQUESTHANDLER_HPP
#define REQUESTHANDLER_HPP

#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include "Cgi.hpp"
#include "Config.hpp"

class RequestHandler {
 private:
  std::string hostPortPair;
  std::string rawRequest;
  ConfigTypes::RouteConfig routeConfig;
  std::string method;
  std::string path;
  std::string version;
  std::map<std::string, std::string> headers;
  std::string body;
  std::string responseContent;
  unsigned int responseStatus;
  long conLen;

  // methods
  void parseRequest();
  void setRouteConfig(ConfigTypes::ServerConfig& serverConfig);
  void getReq(void);
  void postReq(void);
  void deleteReq(void);
  void getCgiHandler(size_t pos_py, size_t pos_query);
  bool availabilityCheck(void);
  // ConfigTypes::RouteConfig& getRouteConfig(
  //     ConfigTypes::ServerConfig& serverConfig);

 public:
  RequestHandler(std::string rawRequest);
  ~RequestHandler();

  void handleRequest(ConfigTypes::ServerConfig& server);
  void printRequest();

  const std::string& getRawRequest() const;
  const std::string& getResponseContent() const;
  const unsigned int getResponseStatus() const;
};

#endif