#ifndef SERVER_HPP
#define SERVER_HPP

#include <poll.h>
#include <set>
#include <string>
#include <vector>
#include "Config.hpp"

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
  std::vector<pollfd> fds;
  std::map<int, size_t> clientToServerIndex;

  void setHostPortPairs(const std::set<std::string>& hostPortPairs);
};

#endif