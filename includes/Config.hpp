#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <map>
#include <set>
#include <string>
#include <vector>
#include <fstream>
#include <stdexcept>
#include <sstream>

namespace ConfigTypes {

  struct RouteConfig {
    std::string root;
    std::string index;
    bool autoindex;
    std::set<std::string> allowedMethods;
    std::string redirect;
  };
  
  struct ServerConfig {
    int port;
    std::string host;
    std::set<std::string> serverNames;
    std::map<std::string, RouteConfig> routes;
  };

}

class Config {
 public:
  Config();
  ~Config();

  void loadFromFile(const std::string& path);
  ConfigTypes::ServerConfig& getServerConfig(const std::string& serverName);

 private:
  std::vector<ConfigTypes::ServerConfig> servers;

};

#endif