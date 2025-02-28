#ifndef SERVER_HPP
#define SERVER_HPP

#include <poll.h>
#include <set>
#include <string>
#include <vector>
#include "Config.hpp"

class Server {
 public:
  Server();
  Server(const Server& other);
  ~Server();

  const Server& operator=(const Server& other);

  static void setNonblocking(int socket);
  void setHostPortPairs(const std::set<std::string>& hostPortPairs);
  void setServer(void);
  void addFD(pollfd fd);
  void serverListen(Config &conf);

 private:
  std::set<std::string> hostPortPairs;
  std::vector<std::string> serverNames;
  std::vector<pollfd> fds;
};

#endif