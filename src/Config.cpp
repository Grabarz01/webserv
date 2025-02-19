#include "Config.hpp"

Config::Config() {};
Config::~Config() {};

void Config::loadFromFile(std::string path) {
  
};

ConfigTypes::ServerConfig& Config::getServerConfig(std::string& serverName) {
  std::vector<ConfigTypes::ServerConfig>::iterator it = servers.begin();
  for (; it != servers.end(); it++) {
    std::set<std::string>::iterator sIt = (*it).serverNames.begin();
    for (; sIt != (*it).serverNames.end(); sIt++) {
      if (*sIt == serverName) {
        return *it;
      }
    }
  }
  return servers.at(0);
};

