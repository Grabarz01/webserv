#include <iostream>
#include <stdexcept>
#include "Config.hpp"
#include "Server.hpp"

int main(int argc, char** argv) {
  if (argc == 1) {
    std::cerr << "Please provide a path to a configuration file" << std::endl;
    return 1;
  }
  if (argc != 2) {
    std::cerr << "Error: invalid configuration file path" << std::endl;
    return 1;
  }

  try {
    Config conf;
    conf.loadFromFile(argv[1]);

    Server server(conf);
    server.setServer();
    server.serverListen();

  } catch (const std::exception& e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }
}
