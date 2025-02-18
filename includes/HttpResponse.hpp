#ifndef HTTPRESPONSE_HPP
#define HTTPRESPONSE_HPP

#include <map>
#include <stdexcept>
#include <string>
#include <sstream>
#include <iostream>

class HttpResponse {
 public:
  HttpResponse();
  ~HttpResponse();

  void setStatus(const unsigned int responseStatus);
  void setBody(const std::string& responseBody);
  const char* getResponseAsString() const;
  void generateResponse();

 private:
  unsigned int status;
  std::string response;
  std::string statusLine;
  std::map<std::string, std::string> headers;
  std::string body;

  void setStatusLine();
  void setHeaders();
};

#endif