#include "HttpResponse.hpp"

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
  if (status == 200)
    response.append(body);
  else
    setBodyForError();
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
  if (!responseBody.empty())
    body = responseBody;
}

void HttpResponse::setBodyForError() {
  std::stringstream ss;

  const char* errorMessage = "Unknown Error";
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
     << "  <style>\n"
     << "    body { font-family: Arial, sans-serif; margin: 40px; line-height: "
        "1.6; }\n"
     << "    .container { max-width: 600px; margin: 0 auto; padding: 20px; "
        "border: 1px solid #ccc; border-radius: 5px; }\n"
     << "    h1 { color: #d33; margin-bottom: 10px; }\n"
     << "    hr { border: 0; border-top: 1px solid #eee; margin: 20px 0; }\n"
     << "  </style>\n"
     << "</head>\n"
     << "<body>\n"
     << "  <div class=\"container\">\n"
     << "    <h1>" << status << " " << errorMessage << "</h1>\n"
     << "    <hr>\n"
     << "    <p>The server cannot process your request.</p>\n";

  switch (status) {
    case 404:
      ss << "    <p>The requested resource could not be found on this "
            "server.</p>\n";
      break;
    case 403:
      ss << "    <p>You don't have permission to access this resource.</p>\n";
      break;
    case 400:
      ss << "    <p>Your browser sent a request that this server could not "
            "understand.</p>\n";
      break;
    case 405:
      ss << "    <p>The method specified in the request is not allowed for the "
            "resource.</p>\n";
      break;
    case 500:
    default:
      ss << "    <p>The server encountered an internal error and was unable to "
            "complete your request.</p>\n";
      break;
  }

  ss << "    <hr>\n"
     << "  </div>\n"
     << "</body>\n"
     << "</html>";

  body = ss.str();
  std::stringstream contentLength;
  contentLength << body.length();
  headers["Content-Length"] = contentLength.str();
  headers["Content-Type"] = "text/html";

  response.append(body);
}