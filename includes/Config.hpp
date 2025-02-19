#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <map>
#include <set>
#include <string>
#include <vector>

namespace ConfigTypes {

  struct RouteConfig {
    std::string root;
    std::string index;
    bool autoindex;
    std::string allowedMethods;
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

  void loadFromFile(std::string path);
  ConfigTypes::ServerConfig& getServerConfig(std::string& serverName);

 private:
  std::vector<ConfigTypes::ServerConfig> servers;
};

#endif