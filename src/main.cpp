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
    Server server;
    server.addPort(8081);
    server.addPort(8080);
    server.setServer();
    server.serverListen();
  } catch (const std::exception& e) {
    std::cerr << "Błąd: " << e.what() << std::endl;
    return 1;
  }
}
