#ifndef HTTPRESPONSE_HPP
#define HTTPRESPONSE_HPP

#include <map>
#include <string>

class HttpResponse {
 public:
  HttpResponse();
  ~HttpResponse();

  void configure(const unsigned int responseStatus,
                 const std::map<unsigned int, std::string>& errorPages,
                 const std::string& responseBody);
  void setStatus(const unsigned int responseStatus);
  void setErrorPages(const std::map<unsigned int, std::string>& errorPages);
  void setBody(const std::string& responseBody);
  const char* getResponseAsString() const;
  void generateResponse();

 private:
  unsigned int status;
  std::string response;
  std::string statusLine;
  std::map<std::string, std::string> headers;
  std::map<unsigned int, std::string> errorPages;
  std::string body;

  void setStatusLine();
  void setHeaders();
  std::string setBodyForError();
};

#endif