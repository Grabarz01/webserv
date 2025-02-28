
#include "Server.hpp"
#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <poll.h>
#include <sys/socket.h>
#include <unistd.h>
#include <algorithm>
#include <cstring>
#include <iostream>
#include <vector>
#include "HttpResponse.hpp"
#include "RequestHandler.hpp"
#include "Config.hpp"

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

Server::Server() {}

Server::Server(const Server& other)
    : hostPortPairs(other.hostPortPairs), serverNames(other.serverNames) {}

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
  this->hostPortPairs = hostPortPairs;
}

void Server::addFD(pollfd fd) {
  fds.push_back(fd);
}

void Server::setServer(void) {
  if (hostPortPairs.empty())
    throw std::runtime_error("Server failed: no host:port provided");
  std::set<std::string>::iterator it;
  for (it = hostPortPairs.begin(); it != hostPortPairs.end(); ++it) {
    std::string host;
    unsigned int port;
    setHostAndPort(*it, host, port);
    // std::cout << "trying to bind: " << host << ":" << port << std::endl;
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0)
      throw std::runtime_error("Socket failed" + std::string(strerror(errno)));

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

void Server::serverListen(Config &conf) {
  fds.reserve(100);
  while (true) {
    int i = poll(&fds[0], fds.size(), 0);
    if (i < 0)
      throw std::runtime_error(std::string("Poll error") +
                               std::string(strerror(errno)));
    std::vector<pollfd>::iterator fd_it = fds.begin();
    while (fd_it != fds.end()) {
      if (fd_it->revents & POLLIN) {
        if (std::distance(fds.begin(), fd_it) < hostPortPairs.size()) {
          // Nowe połączenie
          int client_fd = accept(fd_it->fd, NULL, NULL);
          if (client_fd >= 0) {
            setNonblocking(client_fd);
            pollfd client_pfd = {client_fd, POLLIN, 0};

            fds.push_back(client_pfd);
            std::cout << "Nowe połączenie na porcie "
                      // << *(ports.begin() + std::distance(fds.begin(), fd_it))
                      << "\n";
          } else
            throw std::runtime_error(std::string("Acceot error") +
                                     std::string(strerror(errno)));
        } else {
          // Dane od klienta
          char buffer[1024];
          std::memset(buffer, 0, sizeof(buffer));

          int bytes_received = recv(fd_it->fd, buffer, sizeof(buffer), 0);
          if (bytes_received <= 0) {
            std::cout << "Klient się rozłączył." << std::endl;
            close(fd_it->fd);
            fd_it = fds.erase(fd_it);
            continue;
          } else {
            std::string reqStr(buffer, bytes_received);
            RequestHandler request(reqStr);
            request.handleRequest(conf);

            HttpResponse response;
            response.setStatus(request.getResponseStatus());
            response.setBody(request.getResponseContent());
            response.generateResponse();
            const char* responseStr = response.getResponseAsString();
            int response_len = std::strlen(responseStr);
            std::cout << "Odebrano: " << reqStr << "\n";
            std::cout << "Wyslano: " << responseStr << "\n";

            send(fd_it->fd, responseStr, response_len, 0);
          }
        }
      }
      ++fd_it;  // Przesuwamy iterator tylko, jeśli nie usunęliśmy elementu
    }
  }
}