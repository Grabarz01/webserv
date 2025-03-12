#include "Cgi.hpp"
#include <unistd.h>
#include <algorithm>
#include <cstring>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

std::string Cgi::extractQuery(std::string& path, size_t pos_query) {
  std::string result = "QUERY_STRING=";
  if (pos_query == std::string::npos)
    return result;
  result += path.substr(pos_query + 1, std::string::npos);
  return result;
}

Cgi::Cgi() : cgi_path(NULL), cgi_script_path(NULL) {
  for (size_t i = 0; i < 7; ++i) {
    cgi_envp[i] = NULL;
  }
}

Cgi::~Cgi() {
  for (size_t i = 0; i < 7; ++i) {
    if (cgi_envp[i] != NULL) {
      delete[] cgi_envp[i];
      cgi_envp[i] = NULL;
    }
  }
  delete cgi_path;
  cgi_path = NULL;
  delete cgi_script_path;
  cgi_script_path = NULL;
}
std::string Cgi::extractPathInfo(std::string& path,
                                 size_t pos_query,
                                 size_t pos_py) {
  std::string result = "PATH_INFO=";
  if (pos_py == std::string::npos)
    return result;
  else if (pos_query != std::string::npos)
    result += path.substr(pos_py + 3, pos_query - pos_py - 3);
  else
    result += path.substr(pos_py + 3, std::string::npos);
  return result;
}
std::string Cgi::extractScriptPath(std::string& path,
                                   size_t pos_query,
                                   size_t pos_py) {
  std::string result;
  result = path.substr(1, pos_py + 2);
  std::cerr << result;
  return result;
}
std::string Cgi::getQuery() {
  return query_string;
}
std::string Cgi::getPath() {
  return path_info;
}

std::string Cgi::getScript(void) {
  return script_path;
}
void Cgi::extractEnv(std::string& path, size_t pos_query, size_t pos_py) {
  this->query_string = extractQuery(path, pos_query);
  this->path_info = extractPathInfo(path, pos_query, pos_py);
  this->script_path = extractScriptPath(path, pos_query, pos_py);
}

void Cgi::setEnvp(std::string& method,
                  std::map<std::string, std::string>& headers) {
  std::string requestMethod = "REQUEST_METHOD=" + method;
  std::string contentLength =
      headers.count("Content-Length")
          ? "CONTENT_LENGTH=" + headers["Content-Length"]
          : "CONTENT_LENGTH=0";
  std::string contentType = headers.count("Content-Type")
                                ? "CONTENT_TYPE=" + headers["Content-Type"]
                                : "CONTENT_TYPE=text/plain";
  std::string serverProtocol = "SERVER_PROTOCOL=HTTP/1.1";

  for (size_t i = 0; i < 6; ++i) {
    if (cgi_envp[i] != NULL) {
      delete[] cgi_envp[i];
    }
  }
  cgi_envp[0] = new char[requestMethod.size() + 1];
  std::strcpy(cgi_envp[0], requestMethod.c_str());
  cgi_envp[1] = new char[contentLength.size() + 1];
  std::strcpy(cgi_envp[1], contentLength.c_str());
  cgi_envp[2] = new char[contentType.size() + 1];
  std::strcpy(cgi_envp[2], contentType.c_str());
  cgi_envp[3] = new char[serverProtocol.size() + 1];
  std::strcpy(cgi_envp[3], serverProtocol.c_str());
  cgi_envp[4] = new char[query_string.size() + 1];
  std::strcpy(cgi_envp[4], query_string.c_str());
  cgi_envp[5] = new char[path_info.size() + 1];
  std::strcpy(cgi_envp[5], path_info.c_str());
  cgi_envp[6] = NULL;
  for (size_t i = 0; i < 6; ++i) {
    std::cerr << "[" << i << "] " << cgi_envp[i] << std::endl;
    std::cerr.flush();
  }
}

void Cgi::setCgiPath(std::string cgi_path) {
  this->cgi_path = new char[cgi_path.size() + 1];
  std::strcpy(this->cgi_path, cgi_path.c_str());
}

void Cgi::setCgiScriptPath() {
  this->cgi_script_path = new char[script_path.size() + 1];
  std::strcpy(this->cgi_script_path, script_path.c_str());
}

char* Cgi::getCgiPath(void) {
  return cgi_path;
}
char** Cgi::getCgiEnvp(void) {
  return cgi_envp;
}
char* Cgi::getScriptPath() {
  return cgi_script_path;
}

void Cgi::runCgi(void) {
  char* args[] = {cgi_path, cgi_script_path, NULL};

  execve(args[1], args, cgi_envp);
  execve(cgi_script_path, args, cgi_envp);
}

void Cgi::readResponse(std::string& responseContent,
                       unsigned int& responseStatus,
                       int* pipe_response) {
  char buffer[1024];
  responseContent = "";
  ssize_t bytesRead;
  while ((bytesRead = read(pipe_response[0], buffer, sizeof(buffer) - 1)) > 0) {
    buffer[bytesRead] = '\0';
    responseContent += buffer;
  }

  if (bytesRead == -1) {
    responseStatus = 500;
    responseContent = "CGI script error";
  }
  close(pipe_response[0]);

  size_t pos = responseContent.find("\r\n\r\n");
  if (pos == std::string::npos) {
    responseStatus = 500;
    responseContent = "Malformed CGI response";
    return;
  }
  std::string cgi_headers = responseContent.substr(0, pos);
  responseContent = responseContent.substr(pos + 4);

  std::istringstream headerStream(cgi_headers);
  std::string line;
  std::map<std::string, std::string> headers_map;
  while (std::getline(headerStream, line) && !line.empty()) {
    if (!line.empty() && *(line.rbegin()) == '\r') {
      line.erase(line.size() - 1);
    }

    if (line.find("Status:") == 0) {
      std::string statusCode = line.substr(7);
      responseStatus = std::atoi(statusCode.c_str());
    }
  }
}
