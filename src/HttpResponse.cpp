#include "HttpResponse.hpp"

HttpResponse::HttpResponse() {};
HttpResponse::~HttpResponse() {};

void HttpResponse::setStatus(const unsigned int responseStatus) {
  this->status = responseStatus;
};

const char* HttpResponse::getResponseAsString() const {
  if (response.empty())
    throw std::runtime_error("Response has not been generated");
  return (response.c_str());
};

void HttpResponse::generateResponse() {
  try {
    setStatusLine();
    setHeaders();
  } catch (std::exception& e) {
    std::cerr << "Exception: " << e.what() << std::endl;
  }

  response = statusLine;
  std::map<std::string, std::string>::iterator it = headers.begin();
  for (; it != headers.end(); it++) {
    response.append(it->first);
    response.append(": ");
    response.append(it->second);
    response.append("\r\n");
  }
  response.append("\r\n");
  response.append(body);
};

void HttpResponse::setStatusLine() {
  if (status == 0)
    throw std::runtime_error("Status code is not set");

  switch (status) {
    case 200:
      statusLine = "HTTP/1.1 200 OK\r\n";
      break;
    case 404:
      statusLine = "HTTP/1.1 404 Not Found\r\n";
      break;
  }
}

void HttpResponse::setHeaders() {
  std::stringstream ss;
  ss << body.length();
  headers["Content-length"] = ss.str();
  headers["Server"] = "Default";
  if (status != 200)
    headers["Connection"] = "close";
}

void HttpResponse::setBody(const std::string& responseBody) {
  if (!responseBody.empty())
    body = responseBody;
}