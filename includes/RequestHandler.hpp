#ifndef REQUESTHANDLER_HPP
#define REQUESTHANDLER_HPP

#include <map>
#include <string>
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

 public:
  RequestHandler(std::string rawRequest);
  ~RequestHandler();

  void handleRequest(ConfigTypes::ServerConfig& server);
  void printRequest();
  void autoIndex();

  const std::string& getRawRequest() const;
  const std::string& getResponseContent() const;
  const unsigned int getResponseStatus() const;
};

#endif