#ifndef SERVER_HPP
#define SERVER_HPP

#include <poll.h>
#include <map>
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
  static volatile bool running;
  static void signalHandler(int signum);
  void cleanup();

  Config& serversConfig;
  std::vector<std::string> hostPortPairs;
  std::vector<std::string> serverNames;
  std::vector<pollfd> pollFds;
  std::map<int, ioSocketData> clientFdToIoSocketData;

  void setHostPortPairs(const std::set<std::string>& hostPortPairs);
  void createPollFd(const pollfd listenFd, std::string& hostPortPair);
  void createIoSocketData(int clientFd, std::string& hostPortPair);
  std::vector<pollfd>::iterator receiveDataFromClient(
      std::vector<pollfd>::iterator pollFdIt);
  std::vector<pollfd>::iterator sendDataToClient(
      std::vector<pollfd>::iterator pollFdIt,
      std::string& response);
  void generateResponse(std::vector<pollfd>::iterator pollFdIt,
                        std::string& response);

  std::string getHeaderValue(const std::string& clientRequest,
                             const std::string& headerParameter,
                             const std::string& endChars);
};

#endif