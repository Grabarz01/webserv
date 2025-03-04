#ifndef SERVER_HPP
#define SERVER_HPP

#include <poll.h>
#include <set>
#include <string>
#include <vector>
#include "Config.hpp"

struct ioSocketData {
  std::string hostPortPair;
  std::string clientRequest;
};

class Server {
 public:
  Server(Config& serversConfig);
  Server(const Server& other);
  ~Server();

  const Server& operator=(const Server& other);

  static void setNonblocking(int socket);
  void setServer(void);
  void addFD(pollfd fd);
  void serverListen();

 private:
  Config& serversConfig;
  std::vector<std::string> hostPortPairs;
  std::vector<std::string> serverNames;
  std::vector<pollfd> pollFds;
  std::map<int, ioSocketData> clientFdToIoSocketData;

  void setHostPortPairs(const std::set<std::string>& hostPortPairs);
  std::string getHeaderValue(const std::string& clientRequest,
                             const std::string& headerParameter,
                             const std::string& endChars);
  void createIoSocketData(int clientFd, size_t hostPortPairIndex);
};

#endif