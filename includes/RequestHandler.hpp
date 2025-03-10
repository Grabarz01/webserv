#ifndef REQUESTHANDLER_HPP
#define REQUESTHANDLER_HPP

#include <map>
#include <string>
#include "Config.hpp"

class RequestHandler {
 private:
  std::string hostPortPair;
  std::string rawRequest;
  std::string route;
  ConfigTypes::RouteConfig routeConfig;
  std::string pathWithRoot;
  std::string method;
  std::string path;
  std::string version;
  std::map<std::string, std::string> headers;
  std::string body;

  unsigned int responseStatus;
  std::string responseContent;
  long conLen;

  // methods
  void parseRequest();
  void setRouteConfig(const ConfigTypes::ServerConfig& serverConfig);
  void getReq(void);
  void postReq(void);
  void deleteReq(void);
  void getCgiHandler(size_t pos_py, size_t pos_query);
  void setPathWithRoot(void);
  void redirect(void);
  void indexCheck(void);
  void uploadfile(void);

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