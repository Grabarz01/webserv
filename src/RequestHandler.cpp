#include "RequestHandler.hpp"
#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string>
#include "Cgi.hpp"
#include <dirent.h>
#include <cstring>   
#include <sstream> 

RequestHandler::RequestHandler(std::string rawRequest)
    : rawRequest(rawRequest), responseStatus(200) {
  parseRequest();
};
RequestHandler::~RequestHandler() {};

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

void RequestHandler::handleRequest(Config& conf) {
  // fjalowiec - czy tu ma byc w hostportpair ip:port a w servername string z
  // zapytania np. Host: localhost:8080 to servername = localhost? jesli tak to
  // mozesz to tu ustawic
  //   hostport = headers["HOST"];
  //   ConfigTypes::ServerConfig serv_conf =
  //       conf.getServerConfig(hostport, hostport);
  //   ConfigTypes::RouteConfig rout_conf = getLocationConfig(
  //       serv_conf);  // fjalowiec - we to zaimplementuj na samym dole
  //       zacząłem
  if (!availabilityCheck(/*fjalowiec - przekazanie allowed methods*/)) {
    responseStatus = 405;
    throw std::runtime_error("Method not Allowed");
  } else if (method == "GET") {
    getReq();
  } else if (method == "POST") {
    postReq();
  } else if (method == "DELETE") {
    deleteReq();
  } else {
    responseStatus = 501;
    throw std::runtime_error("Method not implemented");
  }
}

const std::string& RequestHandler::getResponseContent() const {
  return responseContent;
}

const unsigned int RequestHandler::getResponseStatus() const {
  return responseStatus;
}

bool RequestHandler::availabilityCheck(void)  // serwerconfig
{
  /*if(server.stack.empty())
          return true;
  if(stack.find(<std::string>method) != set.end())
          return true
  */
  return true;
}

void RequestHandler::getReq(void) {
  size_t pos_py = path.find(".py");
  size_t pos_query = path.find("?");
  if (pos_py == std::string::npos ||
      pos_query != std::string::npos && pos_query < pos_py)
    ;
  else {
    getCgiHandler(pos_py, pos_query);
    return;
  }

  path.erase(path.begin());
  std::ifstream file(path.c_str());
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

/*cases:
content-size = 0
no content-size
*/
void RequestHandler::postReq() {
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
  if (path.empty()) {
    responseStatus = 404;
    return;
  }
  if (path[0] == '/') {
    path.erase(path.begin());
  }
  std::cout << path << std::endl;
  if (remove(path.c_str()) == 0) {
    responseStatus = 200;
    responseContent = "File deleted successfully";
    conLen = 26;
    return;
  } else {
    if (access(path.c_str(), F_OK) != 0) {
      responseStatus = 404;
      return;
    }
    if (access(path.c_str(), W_OK) != 0) {
      responseStatus = 403;
      return;
    }
  }
}

// najelpiej bedzie jak zwroci referencje do zmiennej gdzie beda zapisane
// zmienne lokacji a jak sa puste to biore z serverconfiga defaultowe dla danego
// servera
ConfigTypes::RouteConfig RequestHandler::getLocationConfig(
    ConfigTypes::ServerConfig& server_conf) {
  std::string cur_location = path;
  ConfigTypes::RouteConfig route_config;

  size_t pos_query = cur_location.find('?');
  size_t pos_py = cur_location.find(".py");

  if (*(cur_location.rbegin()) != '/')
    ;
  else if (pos_py != std::string::npos) {
    cur_location = cur_location.substr(0, pos_py);
    cur_location = cur_location.substr(0, cur_location.find_last_of("/"));
  } else
    cur_location =
        ((pos_query != std::string::npos) ? (cur_location.substr(0, pos_query))
                                          : cur_location);

  // masz aktualna lokacje chyba git i ja tylko sprawdzic
  return route_config;
}

void RequestHandler::autoIndex()
{
	char dirPath[] = "/";
    DIR* dir = opendir(dirPath);
    std::string responseContent;

    if (dir == NULL) {
        responseContent += "<html><body><h2>Error: Directory not found or cannot be opened.</h2></body></html>";
        responseStatus = 404;
		return;
    }

    responseContent += "<html><body><h2>Directory Listing: " + std::string(dirPath) + "</h2><ul>";


    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        std::string entryName = entry->d_name;

        if (entryName == "." || entryName == "..") {
            continue;
        }

        if (entry->d_type == DT_DIR) {
            responseContent += "<li><a href=\"" + entryName + "/\">" + entryName + "/</a></li>";
        } else {  
            responseContent += "<li><a href=\"" + entryName + "\">" + entryName + "</a></li>";
        }
    }

    responseContent += "</ul></body></html>";
	responseStatus = 200;
    closedir(dir);
}
