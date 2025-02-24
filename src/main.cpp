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
#include "Server.hpp"
#include "Config.hpp"

int main() {
  try {
    Config conf;
    conf.loadFromFile("example_files/webserv.conf");
    
    Server server;
    std::set<unsigned int> ports = conf.getPorts();
    std::set<unsigned int>::const_iterator it = ports.begin();
    for (; it != ports.end(); it++) {
      // std::cout << *it << std::endl;
      server.addPort(*it);
    }
    server.setServer();
    server.serverListen();
  } catch (const std::exception& e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }
}
