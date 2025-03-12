#include "Server.hpp"

#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <algorithm>
#include <cerrno>
#include <csignal>
#include <cstring>
#include <iostream>
#include <sstream>
#include "HttpResponse.hpp"
#include "RequestHandler.hpp"

namespace {
void setHostAndPort(const std::string& hostPortPair,
                    std::string& host,
                    unsigned int& port) {
  size_t delimiterPos = hostPortPair.find(':');
  if (delimiterPos == std::string::npos) {
    throw std::runtime_error("Invalid host:port format: " + hostPortPair);
  }

  host = hostPortPair.substr(0, delimiterPos);
  std::string portStr = hostPortPair.substr(delimiterPos + 1);

  std::istringstream iss(portStr);
  if (!(iss >> port) || !iss.eof()) {
    throw std::runtime_error("Invalid port in: " + hostPortPair);
  }

  if (port > 65535) {
    throw std::runtime_error("Port out of range (0-65535): " + portStr);
  }
}
}  // namespace

volatile bool Server::running = true;

void Server::signalHandler(int signum) {
  if (signum == SIGINT || signum == SIGTERM) {
    std::cout << "\nReceived signal " << signum << ", shutting down server..."
              << std::endl;
    running = false;
  }
}

void Server::cleanup() {
  std::vector<pollfd>::iterator it = pollFds.begin();
  for (; it != pollFds.end(); ++it) {
    if (it->fd >= 0) {
      close(it->fd);
    }
  }
  pollFds.clear();
  clientFdToIoSocketData.clear();
}

Server::Server(Config& serversConfig) : serversConfig(serversConfig) {
  setHostPortPairs(serversConfig.getHostPortPairsForConfig());
}

Server::Server(const Server& other)
    : serversConfig(other.serversConfig),
      hostPortPairs(other.hostPortPairs),
      serverNames(other.serverNames) {}

Server::~Server() {}

const Server& Server::operator=(const Server& other) {
  if (&other == this)
    return *this;
  this->hostPortPairs = other.hostPortPairs;
  this->serverNames = other.serverNames;
  return (*this);
}

void Server::setNonblocking(int socket) {
  int flags = fcntl(socket, F_GETFL, 0);
  fcntl(socket, F_SETFL, flags | O_NONBLOCK);
}

void Server::setHostPortPairs(const std::set<std::string>& hostPortPairs) {
  std::set<std::string>::iterator it = hostPortPairs.begin();
  for (; it != hostPortPairs.end(); it++)
    this->hostPortPairs.push_back(*it);
}

void Server::addFD(pollfd fd) {
  pollFds.push_back(fd);
}

void Server::setServer(void) {
  if (hostPortPairs.empty())
    throw std::runtime_error("Server failed: no host:port provided");
  std::vector<std::string>::iterator it;
  for (it = hostPortPairs.begin(); it != hostPortPairs.end(); ++it) {
    std::string host;
    unsigned int port;
    setHostAndPort(*it, host, port);

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0)
      throw std::runtime_error("Socket failed" + std::string(strerror(errno)));
    createIoSocketData(server_fd, *it);

    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) <
        0) {
      throw std::runtime_error("setsockopt(SO_REUSEADDR) failed: " +
                               std::string(strerror(errno)));
    }
    sockaddr_in address;
    std::memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    inet_pton(AF_INET, host.c_str(), &address.sin_addr);
    address.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
      throw std::runtime_error(std::string("Bind failed: ") +
                               std::string(strerror(errno)));
    }

    if (listen(server_fd, 10) < 0)
      throw std::runtime_error(std::string("Listen failed: ") +
                               std::string(strerror(errno)));

    setNonblocking(server_fd);
    pollfd pfd = {server_fd, POLLIN, 0};
    this->addFD(pfd);
  }
  std::cout << "Server listens for that poor messages from u" << std::endl;
}

void Server::serverListen() {
  std::signal(SIGINT, Server::signalHandler);
  std::signal(SIGTERM, Server::signalHandler);

  // pollFds = listenFds + clientFds
  pollFds.reserve(100);
  while (running) {
    if (poll(&pollFds[0], pollFds.size(), 100) < 0) {
      if (errno == EINTR)
        continue;
      throw std::runtime_error(std::string("Poll error: ") +
                               std::string(strerror(errno)));
    }
    std::vector<pollfd>::iterator pollFdIt = pollFds.begin();

    while (pollFdIt != pollFds.end()) {
      if (pollFdIt->revents & POLLIN) {
        if (std::distance(pollFds.begin(), pollFdIt) < hostPortPairs.size()) {
          createPollFd(*pollFdIt,
                       clientFdToIoSocketData[pollFdIt->fd].hostPortPair);
        } else {
          pollFdIt = receiveDataFromClient(pollFdIt);
          continue;
        }
      }
      if (pollFdIt->revents & POLLOUT) {
        pollFdIt = sendDataToClient(pollFdIt);
        continue;
      }
      ++pollFdIt;
    }
  }
  cleanup();
  std::cout << "Server has been shut down gracefully." << std::endl;
}

void Server::createPollFd(const pollfd listenFd, std::string& hostPortPair) {
  int clientFd = accept(listenFd.fd, NULL, NULL);
  if (clientFd < 0)
    throw std::runtime_error(std::string("Accept error") +
                             std::string(strerror(errno)));
  setNonblocking(clientFd);
  pollfd clientPfd = {clientFd, POLLIN | POLLOUT, 0};
  pollFds.push_back(clientPfd);

  createIoSocketData(clientFd, hostPortPair);

  std::cout << "**NEW CONNECTION for host:port " << hostPortPair << "\n";
}

void Server::createIoSocketData(int clientFd, std::string& hostPortPair) {
  ioSocketData ioSocketData;
  ioSocketData.hostPortPair = hostPortPair;
  ioSocketData.clientRequest = "";
  clientFdToIoSocketData[clientFd] = ioSocketData;
}

std::vector<pollfd>::iterator Server::receiveDataFromClient(
    std::vector<pollfd>::iterator pollFdIt) {
  char buffer[1024];
  std::memset(buffer, 0, sizeof(buffer));
  int bytes_received = recv(pollFdIt->fd, buffer, sizeof(buffer), 0);

  if (bytes_received <= 0) {
    if (bytes_received < 0)
      std::cerr << "Warning: Recv fault: " + std::string(strerror(errno))
                << std::endl;
    std::cout << "Connection with "
              << clientFdToIoSocketData.at(pollFdIt->fd).hostPortPair
              << " closed by client\n"
              << std::endl;
    close(pollFdIt->fd);
    clientFdToIoSocketData.erase(pollFdIt->fd);
    return pollFds.erase(pollFdIt);
  } else {
    std::string clientRequest(buffer, bytes_received);
    clientFdToIoSocketData.at(pollFdIt->fd).clientRequest += clientRequest;
    std::cout << "**NEW REQUEST RECEIVED from "
              << clientFdToIoSocketData.at(pollFdIt->fd).hostPortPair
              << std::endl;
    if (!(pollFdIt->events & POLLOUT))
      pollFdIt->events |= POLLOUT;
    return ++pollFdIt;
  }
}

std::vector<pollfd>::iterator Server::sendDataToClient(
    std::vector<pollfd>::iterator pollFdIt) {
  ioSocketData& socket = clientFdToIoSocketData.at(pollFdIt->fd);
  if (socket.clientRequest.find("\r\n\r\n") == std::string::npos)
    return ++pollFdIt;

  std::string serverName =
      getHeaderValue(socket.clientRequest, "Host: ", ":\n");
  std::cout << "Getting config for: " << socket.hostPortPair
            << " and serverName " << serverName << std::endl;
  ConfigTypes::ServerConfig server =
      serversConfig.getServerConfig(socket.hostPortPair, serverName);
  RequestHandler request(socket.clientRequest);
  request.handleRequest(server);

  HttpResponse response;
  response.configure(request.getResponseStatus(), server.errorPages,
                     request.getResponseContent());
  response.generateResponse(request.getResponseHeaders());

  const char* responseStr = response.getResponseAsString();
  int responseLen = std::strlen(responseStr);
  int sent = send(pollFdIt->fd, responseStr, responseLen, 0);

  std::stringstream ss(responseStr);
  std::string status;
  ss >> status >> status;
  std::cout << "**REQUEST RESPONDED with status " << status << "\n"
            << std::endl;
  // std::cout << "Content(" << responseLen <<"): \n" << responseStr <<
  // std::endl;

  bool keepAlive = (socket.clientRequest.find("Connection: keep-alive") !=
                    std::string::npos);
  if (!keepAlive) {
    close(pollFdIt->fd);
    clientFdToIoSocketData.erase(pollFdIt->fd);
    return pollFds.erase(pollFdIt);
  } else {
    socket.clientRequest.clear();
    pollFdIt->events = POLLIN;
    return ++pollFdIt;
  }
}

std::string Server::getHeaderValue(const std::string& clientRequest,
                                   const std::string& headerParameter,
                                   const std::string& endChars) {
  size_t headerParPos = clientRequest.find(headerParameter);
  if (headerParPos == std::string::npos)
    throw std::runtime_error("Server failure: parameter " + headerParameter +
                             " not found in a request header");
  size_t startPos = headerParPos + headerParameter.size();
  size_t endPos = clientRequest.find_first_of(endChars, startPos);
  return (clientRequest.substr(startPos, endPos - startPos));
}
