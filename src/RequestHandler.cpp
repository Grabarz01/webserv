#include "RequestHandler.hpp"

RequestHandler::RequestHandler(std::string rawRequest)
    : rawRequest(rawRequest) {
  parseRequest();
};
RequestHandler::~RequestHandler() {};

void RequestHandler::parseRequest() {
  std::istringstream req(rawRequest);
  req >> this->method >> this->path >> this->version;

  std::string header_line;
  std::getline(req, header_line);
  while (std::getline(req, header_line) && header_line != "\r") {
    std::istringstream header_stream(header_line);
    std::string key;
    if (std::getline(header_stream, key, ':')) {
      std::string value;
      std::getline(header_stream, value);
      headers[key] = value.substr(1);
    }
  }

  if (headers.find("Content-Length") != headers.end()) {
    std::string body_line;
    long conLen = atoi(headers["Content-Length"].c_str()) - 2;
    while (std::getline(req, body_line) || this->body.length() != conLen) {
      this->body.append(body_line);
      this->body.append("\n");
    }
    // std::cout << body.length() << std::endl;
  }
}

const std::string& RequestHandler::getRawRequest() const {
  return rawRequest;
}

void RequestHandler::printRequest() {
  std::cout << method << " " << path << " " << version << "\n";
  std::map<std::string, std::string>::iterator it = headers.begin();
  for (; it != headers.end(); it++) {
    std::cout << it->first << ": " << it->second << "\n";
  }
  std::cout << "\n";
  std::cout << body << std::endl;
}

void RequestHandler::handleRequest() {
  if (method == "GET") {
    std::ifstream file(path.c_str());
    if (!file.is_open())
      throw std::runtime_error("cannot open file");

    std::stringstream buffer;
    buffer << file.rdbuf();
    responseContent = buffer.str();
    file.close();
  } else {
    // TODO: POST, DELETE
    throw std::runtime_error("Method not implemented");
  }
}

const std::string& RequestHandler::getResponseContent() const {
  if (!responseContent.empty())
    return responseContent;
  else
    throw std::runtime_error("Reponse content is empty");
}
