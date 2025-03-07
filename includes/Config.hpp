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
  std::set<std::string> allowedMethods;
  std::vector<std::string> cgiAllowedExtensions;
  std::vector<std::string> redirect;
  std::string maxBodySize;
};

struct ServerConfig {
  std::set<std::string> hostPortPairs;
  std::set<std::string> serverNames;
  std::string cgiPath;
  std::map<unsigned int, std::string> errorPages;
  RouteConfig defaultRoute;
  std::map<std::string, RouteConfig> routes;
};
}  // namespace ConfigTypes

class Config {
 public:
  void loadFromFile(const std::string& path);
  ConfigTypes::ServerConfig& getServerConfig(const std::string& hostPortPair,
                                             const std::string& serverName);
  const std::set<std::string>& getHostPortPairsForConfig() const;

 private:
  std::vector<ConfigTypes::ServerConfig> servers;
  std::set<std::string> hostPortPairsForConfig;

  void setHostPortPairsForConfig();
};

#endif