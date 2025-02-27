#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <map>
#include <set>
#include <string>
#include <vector>
#include <fstream>
#include <stdexcept>
#include <sstream>
#include <cstdlib>
#include <iostream>

namespace ConfigTypes {

  struct RouteConfig {
    std::string root;
    std::string index;
    bool autoindex;
    std::set<std::string> allowedMethods;
    std::vector<std::string> redirect;
  };
  
  struct ServerConfig {
    std::set<unsigned int> ports;
    std::string host;
    std::set<std::string> serverNames;
    RouteConfig defaultRoute;
    std::map<std::string, RouteConfig> routes;

    ServerConfig();
  };
}

class Config {
 public:
  void loadFromFile(const std::string& path);
  ConfigTypes::ServerConfig& getServerConfig(const std::string& serverName, const std::string& port);
  const std::set<unsigned int>& getPorts() const;

 private:
  std::vector<ConfigTypes::ServerConfig> servers;
  std::set<unsigned int> ports;

  void setPorts();

};

#endif