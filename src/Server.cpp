
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
#include "RequestHandler.hpp"
#include "HttpResponse.hpp"

Server::Server() {}

Server::Server(const Server& other) : ports(other.ports) {}

Server::~Server() {}

const Server& Server::operator=(const Server& other) {
  if (&other == this)
    return *this;
  this->ports = other.ports;
  this->server_names = other.server_names;
  return (*this);
}

void Server::setNonblocking(int socket) {
  int flags = fcntl(socket, F_GETFL, 0);
  fcntl(socket, F_SETFL, flags | O_NONBLOCK);
}

void Server::addPort(int port) {
  ports.push_back(port);
}

void Server::addFD(pollfd fd) {
  fds.push_back(fd);
}

void Server::setServer(void) {
  std::vector<int>::iterator it;
  for (it = ports.begin(); it != ports.end(); ++it) {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0)
      throw std::runtime_error(std::string("Socket failed") +
                               std::string(strerror(errno)));

    sockaddr_in address;
    std::memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    inet_pton(AF_INET, "127.0.0.1", &address.sin_addr);
    address.sin_port = htons(*it);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
      throw std::runtime_error(std::string("Bind failedd") +
                               std::string(strerror(errno)));
    }

    if (listen(server_fd, 10) < 0)
      throw std::runtime_error(std::string("Listen failed") +
                               std::string(strerror(errno)));

    setNonblocking(server_fd);
    pollfd pfd = {server_fd, POLLIN, 0};
    this->addFD(pfd);
  }
  std::cout << "Server listens for that poor messages from u" << std::endl;
}

void Server::serverListen(void) {
  fds.reserve(100);
  while (true) {
    int i = poll(&fds[0], fds.size(), 0);
	if (i < 0) throw std::runtime_error(
        std::string("Poll error") + std::string(strerror(errno)));
    std::vector<pollfd>::iterator fd_it = fds.begin();
    while (fd_it != fds.end()) {
      if (fd_it->revents & POLLIN) {
        if (std::distance(fds.begin(), fd_it) < ports.size()) {
          // Nowe połączenie
          int client_fd = accept(fd_it->fd, NULL, NULL);
          if (client_fd >= 0) {
            setNonblocking(client_fd);
            pollfd client_pfd = {client_fd, POLLIN, 0};

            fds.push_back(client_pfd);
            std::cout << "Nowe połączenie na porcie "
                      << *(ports.begin() + std::distance(fds.begin(), fd_it))
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
            fd_it = fds.erase(
                fd_it);
            continue;
          } else {
			std::string reqStr(buffer, bytes_received);
			RequestHandler request(reqStr);
			request.handleRequest();

			HttpResponse response;
			response.setStatus(request.getResponseStatus());
			response.setBody(request.getResponseContent());
			response.generateResponse();
			const char *responseStr = response.getResponseAsString();
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