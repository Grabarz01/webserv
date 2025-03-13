#include "Cgi.hpp"
#include <unistd.h>
#include <algorithm>
#include <cstring>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

std::string Cgi::extractQuery(std::string& path) {
	std::string result = "QUERY_STRING=";
	if (posQuery == std::string::npos)
    return result;
	result += path.substr(posQuery + 1, std::string::npos);
	return result;
}

Cgi::Cgi() : cgiPath(NULL), cgiScriptPath(NULL) {
	for (size_t i = 0; i < 7; ++i) {
		cgiEnvp[i] = NULL;
	}
}

Cgi::~Cgi() {
	for (size_t i = 0; i < 7; ++i) {
		if (cgiEnvp[i] != NULL) {
			delete[] cgiEnvp[i];
			cgiEnvp[i] = NULL;
		}
	}
	delete cgiPath;
	cgiPath = NULL;
	delete cgiScriptPath;
	cgiScriptPath = NULL;
}

std::string Cgi::extractPathInfo(std::string& path, size_t size) {
	std::string result = "PATH_INFO=";
	if (posQuery != std::string::npos)
	result += path.substr(posCgiExt + size, posQuery - posCgiExt - size);
	else
    result += path.substr(posCgiExt + size, std::string::npos);
	return result;
}

std::string Cgi::extractScriptPath(std::string& path, size_t size) {
  std::string result;
  result = path.substr(1, posCgiExt + size - 1);
  std::cerr << result;
  return result;
}
std::string Cgi::getQuery() {
	return queryString;
}
std::string Cgi::getPath() {
	return pathInfo;
}

std::string Cgi::getScript(void) {
	return scriptPath;
}
void Cgi::extractEnvFromPath(std::string& path) {
	size_t size = cgiExtension == CGI_PHP ? 4 : 3;
	this->queryString = extractQuery(path);
	this->pathInfo = extractPathInfo(path, size);
	this->scriptPath = extractScriptPath(path, size);
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
    if (cgiEnvp[i] != NULL) {
      delete[] cgiEnvp[i];
    }
  }
  cgiEnvp[0] = new char[requestMethod.size() + 1];
  std::strcpy(cgiEnvp[0], requestMethod.c_str());
  cgiEnvp[1] = new char[contentLength.size() + 1];
  std::strcpy(cgiEnvp[1], contentLength.c_str());
  cgiEnvp[2] = new char[contentType.size() + 1];
  std::strcpy(cgiEnvp[2], contentType.c_str());
  cgiEnvp[3] = new char[serverProtocol.size() + 1];
  std::strcpy(cgiEnvp[3], serverProtocol.c_str());
  cgiEnvp[4] = new char[queryString.size() + 1];
  std::strcpy(cgiEnvp[4], queryString.c_str());
  cgiEnvp[5] = new char[pathInfo.size() + 1];
  std::strcpy(cgiEnvp[5], pathInfo.c_str());
  cgiEnvp[6] = NULL;
  for (size_t i = 0; i < 6; ++i) {
    std::cerr << "[" << i << "] " << cgiEnvp[i] << std::endl;
    std::cerr.flush();
  }
}

void Cgi::setCgiPath(std::string cgi_path) {
  this->cgiPath = new char[cgi_path.size() + 1];
  std::strcpy(this->cgiPath, cgi_path.c_str());
}

void Cgi::setCgiScriptPath() {
  this->cgiScriptPath = new char[scriptPath.size() + 1];
  std::strcpy(this->cgiScriptPath, scriptPath.c_str());
}

char* Cgi::getCgiPath(void) {
  return cgiPath;
}
char** Cgi::getCgiEnvp(void) {
  return cgiEnvp;
}
char* Cgi::getScriptPath() {
  return cgiScriptPath;
}

void Cgi::runCgi(void) {
  char* args[] = {cgiPath, cgiScriptPath, NULL};

  execve(args[1], args, cgiEnvp);
  execve(cgiScriptPath, args, cgiEnvp);
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

int Cgi::checkCgi(std::string pathWithRoot,
                  std::string cgiPathPython,
                  std::string cgiPathPhp,
                  std::vector<std::string> ext) {
  if (ext.empty())
    return 403;
  if (cgiPathPython.empty() && cgiPathPhp.empty())
    return 403;

  posPhp = pathWithRoot.find(".php");
  posPython = pathWithRoot.find(".py");
  posQuery = pathWithRoot.find("?");

  if (posPhp > posQuery && posPython > posQuery)
    return 400;
  if (posPhp != std::string::npos && posPython != std::string::npos) {
    cgiExtension = posPython < posPhp ? CGI_PYTHON : CGI_PHP;
  } else if (posPhp != std::string::npos || posPython != std::string::npos) {
    cgiExtension = posPhp != std::string::npos ? CGI_PHP : CGI_PYTHON;
  } else
    return 400;

  posCgiExt = cgiExtension == CGI_PYTHON ? posPython : posPhp;
  if (cgiExtension == CGI_PYTHON) {
    if (posCgiExt > posQuery || cgiPathPython.empty() ||
        std::find(ext.begin(), ext.end(), ".py") == ext.end())
      return 400;
  } else {
    if (posCgiExt > posQuery || cgiPathPhp.empty() ||
        std::find(ext.begin(), ext.end(), ".php") == ext.end())
      return 400;
  }
  return 200;
}
