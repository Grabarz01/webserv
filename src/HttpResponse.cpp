#include "HttpResponse.hpp"

#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>

struct HttpError {
  unsigned int code;
  const char* message;
};

static const HttpError httpErrors[] = {{200, "OK"},
                                       {400, "Bad Request"},
                                       {403, "Forbidden"},
                                       {404, "Not Found"},
                                       {405, "Method Not Allowed"},
                                       {500, "Internal Server Error"}};

static const size_t httpErrorsCount =
    sizeof(httpErrors) / sizeof(httpErrors[0]);

HttpResponse::HttpResponse() {};
HttpResponse::~HttpResponse() {};

void HttpResponse::configure(
    const unsigned int responseStatus,
    const std::map<unsigned int, std::string>& errorPages,
    const std::string& responseBody) {
  setStatus(responseStatus);
  setErrorPages(errorPages);
  setBody(responseBody);
}

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
  response.append("\r\n\r\n");
};

void HttpResponse::setStatusLine() {
  if (status == 0)
    throw std::runtime_error("Status code is not set");

  const char* msg = "Internal Server Error";  // default
  for (size_t i = 0; i < httpErrorsCount; i++) {
    if (httpErrors[i].code == status) {
      msg = httpErrors[i].message;
      break;
    }
  }
  std::stringstream ss;
  ss << "HTTP/1.1 " << status << " " << msg << "\r\n";
  statusLine = ss.str();
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
  if (status == 200 && !responseBody.empty())
    body = responseBody;
  if (status != 200)
    body = setBodyForError();
}

std::string HttpResponse::setBodyForError() {
  if (!errorPages.empty() && errorPages.find(status) != errorPages.end()) {
    std::string path = errorPages.at(status);
    std::ifstream file(path.c_str());
    if (file.is_open()) {
      std::stringstream buffer;
      buffer << file.rdbuf();
      file.close();
      return buffer.str();
    } else {
      std::cerr << "File at " + path +
                       " does not exist. Using default error page."
                << std::endl;
    }
  }

  std::stringstream ss;

  std::string errorMessage = "Unknown Error";
  for (size_t i = 0; i < httpErrorsCount; i++) {
    if (httpErrors[i].code == status) {
      errorMessage = httpErrors[i].message;
      break;
    }
  }

  ss << "<!DOCTYPE html>\n"
     << "<html>\n"
     << "<head>\n"
     << "  <title>" << status << " " << errorMessage << "</title>\n"
     << "</head>\n"
     << "<body>\n"
     << "    <h1>" << status << " " << errorMessage << "</h1>\n"
     << "    <hr>\n"
     << "    <p>The server cannot process your request.</p>\n"
     << "</body>\n"
     << "</html>";

  return (ss.str());
}

void HttpResponse::setErrorPages(
    const std::map<unsigned int, std::string>& errorPages) {
  this->errorPages = errorPages;
}