#include "RequestHandler.hpp"

#include <dirent.h>
#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include "Cgi.hpp"

namespace {

void copyDefaultValuesToRouteConfig(
    ConfigTypes::RouteConfig& routeConfig,
    const ConfigTypes::ServerConfig& serverConfig) {
  if (routeConfig.root.empty())
    routeConfig.root = serverConfig.defaultRoute.root;
  if (routeConfig.index.empty())
    routeConfig.index = serverConfig.defaultRoute.index;
  if (routeConfig.allowedMethods.empty())
    routeConfig.allowedMethods = serverConfig.defaultRoute.allowedMethods;
  if (routeConfig.cgiAllowedExtensions.empty())
    routeConfig.cgiAllowedExtensions =
        serverConfig.defaultRoute.cgiAllowedExtensions;
  if (routeConfig.redirect.empty())
    routeConfig.redirect = serverConfig.defaultRoute.redirect;
  if (routeConfig.maxBodySize.empty())
    routeConfig.maxBodySize = serverConfig.defaultRoute.maxBodySize;
  if (routeConfig.autoindex.empty() &&
      !serverConfig.defaultRoute.autoindex.empty())
    routeConfig.autoindex = serverConfig.defaultRoute.autoindex;
  else if (routeConfig.autoindex.empty())
    routeConfig.autoindex = "off";
  if (routeConfig.uploadPath.empty())
    routeConfig.uploadPath = serverConfig.defaultRoute.uploadPath;
}

}  // namespace

RequestHandler::RequestHandler(std::string rawRequest)
    : rawRequest(rawRequest), responseStatus(200) {
  parseRequest();
};
RequestHandler::~RequestHandler() {};

const std::map<std::string, std::string>& RequestHandler::getResponseHeaders(
    void) {
  return this->responseHeaders;
}
void RequestHandler::parseRequest() {
  std::istringstream req(rawRequest);
  req >> this->method >> this->path >> this->version;
  std::cerr << "method: " << method << std::endl;
  std::string header_line;
  std::getline(req, header_line);
  while (std::getline(req, header_line) && header_line != "\r") {
    std::istringstream header_stream(header_line);
    std::string key;
    if (std::getline(header_stream, key, ':')) {
      std::string value;
      std::getline(header_stream, value);
      value.erase(0, value.find_first_not_of(" \t\r\n"));
      value.erase(value.find_last_not_of(" \t\r\n") + 1);
      headers[key] = value;
    }
  }

  if (headers.find("Transfer-Encoding") != headers.end() &&
      headers["Transfer-Encoding"] == "chunked") {
    std::string chunk_size_str;
    while (std::getline(req, chunk_size_str) && chunk_size_str != "\r") {
      if (!chunk_size_str.empty() &&
          chunk_size_str[chunk_size_str.size() - 1] == '\r') {
        chunk_size_str.erase(chunk_size_str.size() - 1);
      }

      int chunk_size = std::strtol(chunk_size_str.c_str(), NULL, 16);
      std::cerr << "Chunk Size: " << chunk_size << std::endl;

      if (chunk_size == 0) {
        break;
      }
      std::string chunk(chunk_size, '\0');
      req.read(&chunk[0], chunk_size);
      body += chunk;

      std::getline(req, chunk_size_str);
    }

    std::ostringstream len_stream;
    len_stream << body.size();
    headers["Content-Length"] = len_stream.str();
  } else if (headers.find("Content-Length") != headers.end()) {
    conLen = std::atoi(headers["Content-Length"].c_str());
    if (conLen > 0) {
      body.resize(conLen);
      req.read(&body[0], conLen);
    }
  }
}

void RequestHandler::setRouteConfig(
    const ConfigTypes::ServerConfig& serverConfig) {
  std::string route = path;
  size_t queryPos = route.find("?");
  if (queryPos != std::string::npos) {
    route = route.substr(0, queryPos);
  }

  std::cout << "receivedRoute: " << route << std::endl;
  while (!route.empty()) {
    // std::cout << "loopRoute: " << route << std::endl;
    if (serverConfig.routes.find(route) != serverConfig.routes.end()) {
      std::map<std::string, ConfigTypes::RouteConfig>::const_iterator it;
      it = serverConfig.routes.find(route);
      std::string routePath = it->first;
      routeConfig = it->second;
      std::cout << "used route: " << routePath << std::endl;
      this->route = route;
      return;
    } else {
      if (route.size() > 1) {
        size_t slashPos = route.find_last_of('/', route.size() - 2);
        route = route.substr(0, slashPos + 1);
      } else
        break;
    }
  }

  if (serverConfig.routes.find("/") != serverConfig.routes.end()) {
    routeConfig = serverConfig.routes.find("/")->second;
    this->route = "/";
    std::cout << "used route: /" << std::endl;
  } else {
    responseStatus = 404;
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

void RequestHandler::handleRequest(ConfigTypes::ServerConfig& server) {
  setRouteConfig(server);
  copyDefaultValuesToRouteConfig(routeConfig, server);
  setPathWithRoot();

  if (!routeConfig.redirect.empty())
    redirect();
  else if (*path.rbegin() == '/') {
    if (!routeConfig.index.empty())
      indexCheck();
    else if (routeConfig.autoindex == "on")
      autoIndex();
  } else if (routeConfig.allowedMethods.find(method) ==
             routeConfig.allowedMethods.end()) {
    responseStatus = 405;
  } else if (method == "GET") {
    getReq();
  } else if (method == "POST") {
    postReq();
  } else if (method == "DELETE") {
    deleteReq();
  } else {
    responseStatus = 501;
  }
}

const std::string& RequestHandler::getResponseContent() const {
  return responseContent;
}

const unsigned int RequestHandler::getResponseStatus() const {
  return responseStatus;
}

void RequestHandler::setPathWithRoot() {
  if (routeConfig.root.empty() || route.empty() || path.empty() ||
      path.find(route, 0) == std::string::npos) {
    return;
  }

  pathWithRoot = routeConfig.root + path.substr(route.size(), path.size());

  std::cout << "pathWithRoot: " << pathWithRoot << std::endl;
}

void RequestHandler::getReq(void) {
  size_t pos_py = pathWithRoot.find(".py");
  size_t pos_query = pathWithRoot.find("?");
  if (pos_py == std::string::npos ||
      pos_query != std::string::npos && pos_query < pos_py)
    ;
  else {
    getCgiHandler(pos_py, pos_query);
    return;
  }

  pathWithRoot.erase(pathWithRoot.begin());
  std::ifstream file(pathWithRoot.c_str());
  if (!file.is_open()) {
    responseStatus = 404;
    return;
  }

  std::stringstream buffer;
  buffer << file.rdbuf();
  responseContent = buffer.str();
  file.close();
}

void RequestHandler::getCgiHandler(size_t pos_py, size_t pos_query) {
  int pipe_response[2];
  if (pipe(pipe_response) == -1) {
    std::cerr << "Fork failed\n";
    responseStatus = 500;
    return;
  }

  pid_t pid = fork();
  if (pid == -1) {
    std::cerr << "Fork failed\n";
    responseStatus = 500;
    return;
  }

  if (pid == 0) {
    close(pipe_response[0]);
    dup2(pipe_response[1], STDOUT_FILENO);

    Cgi cgi;
    cgi.extractEnv(path, pos_query, pos_py);
    std::string requestMethod = "REQUEST_METHOD=GET";
    std::string serverProtocol = "SERVER_PROTOCOL=HTTP/1.1";
    char* envp[] = {const_cast<char*>(requestMethod.c_str()),
                    const_cast<char*>(cgi.getPath().c_str()),
                    const_cast<char*>(cgi.getQuery().c_str()), NULL};
    char python_path[] = "/usr/bin/python3";
    std::string str = "/home/filip/webserv2" + cgi.getScript();

    char* script_path = const_cast<char*>(str.c_str());
    std::cerr << "here" << script_path << "?";
    char* args[] = {python_path, script_path, NULL};
    std::cerr << "Attempting to exec: " << python_path << " " << script_path
              << std::endl;
    execve(args[1], args, envp);
    std::cout << args[0] << std::endl;
    perror("Execve failed\n");
    exit(1);
  } else {
    // server
    close(pipe_response[1]);

    char buffer[1024];
    responseContent = "";
    ssize_t bytesRead;
    while ((bytesRead = read(pipe_response[0], buffer, sizeof(buffer) - 1)) >
           0) {
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
    waitpid(pid, NULL, 0);
  }
}

void RequestHandler::postReq() {
  if (atoi(headers["Content-Length"].c_str()) >
      atoi(routeConfig.maxBodySize.c_str())) {
    responseStatus = 413;
    return;
  }
  if (headers["Content-Type"].find("multipart/form-data") !=
      std::string::npos) {
    uploadfile();
    return;
  }
  size_t pos_py = path.find(".py");
  size_t pos_query = path.find("?");
  if (pos_py == std::string::npos ||
      pos_query != std::string::npos && pos_query < pos_py) {
    throw std::runtime_error("Unsuported Media Type");
  }
  Cgi cgi;
  cgi.extractEnv(path, pos_query, pos_py);

  int pipe_input[2];
  int pipe_response[2];
  if (pipe(pipe_input) == -1 || pipe(pipe_response) == -1) {
    std::cerr << "Pipe creation failed\n";
    responseStatus = 500;
    return;
  }

  pid_t pid = fork();
  if (pid == -1) {
    std::cerr << "Fork failed\n";
    responseStatus = 500;
    return;
  }

  if (pid == 0) {
    // CGI
    close(pipe_input[1]);
    close(pipe_response[0]);

    dup2(pipe_input[0], STDIN_FILENO);
    dup2(pipe_response[1], STDOUT_FILENO);
    close(pipe_input[0]);
    close(pipe_response[1]);

    std::string requestMethod = "REQUEST_METHOD=POST";
    std::string contentLength =
        headers.count("Content-Length")
            ? "CONTENT_LENGTH=" + headers["Content-Length"]
            : "CONTENT_LENGTH=0";
    std::string contentType = headers.count("Content-Type")
                                  ? "CONTENT_TYPE=" + headers["Content-Type"]
                                  : "CONTENT_TYPE=text/plain";
    std::string serverProtocol = "SERVER_PROTOCOL=HTTP/1.1";
    char* envp[] = {const_cast<char*>(requestMethod.c_str()),
                    const_cast<char*>(contentLength.c_str()),
                    const_cast<char*>(contentType.c_str()),
                    const_cast<char*>(cgi.getPath().c_str()),
                    const_cast<char*>(cgi.getQuery().c_str()),
                    NULL};
    char python_path[] = "/usr/bin/python3";
    std::string str = "/home/filip/webserv2" + cgi.getScript();

    char* script_path = const_cast<char*>(str.c_str());
    std::cerr << "here" << script_path << "?";
    char* args[] = {python_path, script_path, NULL};
    std::cerr << "Attempting to exec: " << python_path << " " << script_path
              << std::endl;
    execve(args[1], args, envp);
    std::cout << args[0] << std::endl;
    perror("Execve failed\n");
    exit(1);
  } else {
    // server
    close(pipe_input[0]);
    close(pipe_response[1]);

    write(pipe_input[1], body.c_str(), body.size());
    close(pipe_input[1]);

    char buffer[1024];
    responseContent = "";
    ssize_t bytesRead;
    while ((bytesRead = read(pipe_response[0], buffer, sizeof(buffer) - 1)) >
           0) {
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
    waitpid(pid, NULL, 0);
  }
}

void RequestHandler::deleteReq(void) {
  if (pathWithRoot.empty()) {
    responseStatus = 404;
    return;
  }
  if (pathWithRoot[0] == '/') {
    pathWithRoot.erase(pathWithRoot.begin());
  }
  std::cout << pathWithRoot << std::endl;
  if (remove(pathWithRoot.c_str()) == 0) {
    responseStatus = 200;
    responseContent = "File deleted successfully";
    conLen = 26;
    return;
  } else {
    if (access(pathWithRoot.c_str(), F_OK) != 0) {
      responseStatus = 404;
      return;
    }
    if (access(pathWithRoot.c_str(), W_OK) != 0) {
      responseStatus = 403;
      return;
    }
  }
}

void RequestHandler::autoIndex() {
  std::cout << "Generating directory listing..." << std::endl;
  std::cout << pathWithRoot << std::endl;
  std::string pathlisting = pathWithRoot.substr(1, std::string::npos);
  DIR* dir = opendir(pathlisting.c_str());

  if (dir == NULL) {
    responseStatus = 404;
    return;
  }

  responseContent =
      "<html><body><h2>Directory Listing: " + std::string(pathlisting) +
      "</h2><ul>";

  struct dirent* entry;
  while ((entry = readdir(dir)) != NULL) {
    std::string entryName = entry->d_name;
    if (entryName == "." || entryName == "..") {
      continue;
    }

    responseContent += "<li><a href=\"" + pathWithRoot + entryName;
    if (entry->d_type == DT_DIR) {
      responseContent += "/\">" + entryName + "/</a></li>";
    } else {
      responseContent += "\">" + entryName + "</a></li>";
    }
  }

  responseContent += "</ul></body></html>";
  closedir(dir);
}

void RequestHandler::redirect(void) {
  if (routeConfig.redirect.size() == 2) {
    int status = atoi(routeConfig.redirect[0].c_str());

    if (status >= 300 && status < 400) {
      responseStatus = 301;
      responseHeaders["Location"] = routeConfig.redirect[1];
    } else {
      responseStatus = 500;
    }
  } else {
    responseStatus = 500;
  }
}

void RequestHandler::indexCheck() {
  pathWithRoot.erase(pathWithRoot.begin());
  pathWithRoot += routeConfig.index;
  std::ifstream file(pathWithRoot.c_str());
  if (!file.is_open()) {
    responseStatus = 404;
    return;
  }

  std::stringstream buffer;
  buffer << file.rdbuf();
  responseContent = buffer.str();
  file.close();
  responseStatus = 200;
}

void RequestHandler::uploadfile(void) {
  if (routeConfig.uploadPath.empty() ||
      routeConfig.uploadPath[routeConfig.uploadPath.size() - 1] != '/') {
    responseStatus = 400;
    return;
  }
  std::string content_type = headers["Content-Type"];
  size_t pos = content_type.find("boundary=");
  std::string boundary = content_type.substr(pos + 9, std::string::npos);
  std::string delimiter = "--" + boundary;
  std::string end_boundary = delimiter + "--";
  responseStatus = 200;

  size_t part_start = body.find(delimiter);
  if (part_start == std::string::npos) {
    responseStatus = 400;
    return;
  }
  size_t body_end = body.find(end_boundary);
  if (body_end == std::string::npos) {
    responseStatus = 400;
    return;
  }

  while (part_start != std::string::npos && part_start != body_end) {
    part_start += delimiter.length();

    size_t disposition_start = body.find("Content-Disposition:", part_start);
    if (disposition_start == std::string::npos) {
      responseStatus = 400;
      return;
    }

    size_t filename_pos = body.find("filename=\"", disposition_start);
    if (filename_pos == std::string::npos) {
      responseStatus = 400;
      return;
    }

    filename_pos += 10;
    size_t filename_end = body.find("\"", filename_pos);
    if (filename_end == std::string::npos) {
      responseStatus = 400;
      return;
    }

    std::string filename =
        body.substr(filename_pos, filename_end - filename_pos);
    if (filename.empty()) {
      responseStatus = 400;
      return;
    }

    size_t file_data_start = body.find("\r\n\r\n", filename_end);
    if (file_data_start == std::string::npos) {
      responseStatus = 400;
      return;
    }
    file_data_start += 4;

    size_t file_data_end = body.find(delimiter, file_data_start);
    if (file_data_end == std::string::npos) {
      responseStatus = 400;
      return;
    }

    file_data_end -= 2;
    filename = routeConfig.uploadPath + filename;
    std::ofstream file(filename.c_str(), std::ios::binary);
    if (!file) {
      std::cerr << "Błąd: nie można otworzyć pliku " << filename << std::endl;
      responseStatus = 403;
      return;
    }
    file.write(body.c_str() + file_data_start, file_data_end - file_data_start);
    file.close();
    part_start = body.find(delimiter, file_data_end);
  }
}