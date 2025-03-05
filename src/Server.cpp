#include "Server.hpp"

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

void Server::serverListen() {
  signal(SIGINT, Server::signalHandler);
  signal(SIGTERM, Server::signalHandler);

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
          // New connection
          size_t listenFdIndex = std::distance(pollFds.begin(), pollFdIt);
          createPollFd(*pollFdIt, hostPortPairs[listenFdIndex]);
        } else {
          pollFdIt = receiveDataFromClient(pollFdIt);
          continue;
        }
      }
      if (pollFdIt->revents & POLLOUT) {
        sendDataToClient(*pollFdIt);
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

  std::cout << "New connection for host:port " << hostPortPair << "\n";
}

void Server::createIoSocketData(int clientFd, std::string& hostPortPair) {
  ioSocketData ioSocketData;
  ioSocketData.hostPortPair = hostPortPair;
  clientFdToIoSocketData[clientFd] = ioSocketData;
}

std::vector<pollfd>::iterator Server::receiveDataFromClient(
    std::vector<pollfd>::iterator pollFdIt) {
  char buffer[1024];
  std::memset(buffer, 0, sizeof(buffer));
  int bytes_received = recv(pollFdIt->fd, buffer, sizeof(buffer), 0);

  if (bytes_received <= 0) {
    std::cout << "Connection closed by client\n" << std::endl;
    close(pollFdIt->fd);
    clientFdToIoSocketData.erase(pollFdIt->fd);
    return pollFds.erase(pollFdIt);
  } else {
    std::string clientRequest(buffer, bytes_received);
    clientFdToIoSocketData.at(pollFdIt->fd).clientRequest += clientRequest;
    std::cout << "Request from "
              << clientFdToIoSocketData.at(pollFdIt->fd).hostPortPair
              << " received" << std::endl;
    if (!(pollFdIt->events & POLLOUT))
      pollFdIt->events |= POLLOUT;
    return ++pollFdIt;
  }
}

void Server::sendDataToClient(pollfd& clientFd) {
  ioSocketData socket = clientFdToIoSocketData.at(clientFd.fd);
  std::string serverName =
      getHeaderValue(socket.clientRequest, "Host: ", ":\n");
  std::cout << "Getting config for: " << socket.hostPortPair
            << " and serverName " << serverName << std::endl;

  ConfigTypes::ServerConfig server =
      serversConfig.getServerConfig(socket.hostPortPair, serverName);
  RequestHandler request(socket.clientRequest);
  request.handleRequest(server);

  HttpResponse response;
  response.setStatus(request.getResponseStatus());
  response.setBody(request.getResponseContent());
  response.generateResponse();

  const char* responseStr = response.getResponseAsString();
  int response_len = std::strlen(responseStr);

  send(clientFd.fd, responseStr, response_len, 0);
  clientFd.events = POLLIN;

  std::cout << "Request responded" << std::endl;
  // std::cout << "Content: \n" << responseStr << "\n";
}

std::string Server::getHeaderValue(const std::string& clientRequest,
                                   const std::string& headerParameter,
                                   const std::string& endChars) {
  size_t headerParPos = clientRequest.find(headerParameter);
  std::cout << "clientRequest: " << clientRequest << std::endl;
  if (headerParPos == std::string::npos)
    throw std::runtime_error("Server failure: parameter " + headerParameter +
                             " not found in a request header");
  size_t startPos = headerParPos + headerParameter.size();
  size_t endPos = clientRequest.find_first_of(endChars, startPos);
  return (clientRequest.substr(startPos, endPos - startPos));
}
