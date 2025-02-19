#include "Config.hpp"

//TODO: copy constructor and =operator
Config::Config() {};
Config::~Config() {};

namespace {
enum ParameterType {
  PARAM_LISTEN,
  PARAM_SERVER_NAMES,
  PARAM_LOCATION,
  PARAM_UNKNOWN,
  PARAM_ROUTE_ALLOWED_METH,
  PARAM_ROUTE_ROOT,
  PARAM_ROUTE_INDEX,
  PARAM_ROUTE_RETURN,
  PARAM_ROUTE_AUTOINDEX
};

ParameterType getParameterType(const std::string& param) {
  if (param == "listen")
    return PARAM_LISTEN;
  else if (param == "serverNames")
    return PARAM_SERVER_NAMES;
  else if (param == "location")
    return PARAM_LOCATION;
  else if (param == "allowedMethods")
    return PARAM_ROUTE_ALLOWED_METH;
  else if (param == "root")
    return PARAM_ROUTE_ROOT;
  else if (param == "index")
    return PARAM_ROUTE_INDEX;
  else if (param == "return")
    return PARAM_ROUTE_RETURN;
  else if (param == "autoindex")
    return PARAM_ROUTE_AUTOINDEX;
  else
    return PARAM_UNKNOWN;
}

//TODO: extensive tests, proper validation
void parseRoute(ConfigTypes::ServerConfig& server,
                std::istringstream& iss,
                std::ifstream& file) {
  ConfigTypes::RouteConfig route;
  std::string routePath;
  iss >> routePath;

  std::string line;
  while (std::getline(file, line) && line.at(line.size() - 1) != '}') {
    std::string parameter;
    std::istringstream routeIss(line);
    routeIss >> parameter;
    if (parameter.empty())
      continue;
    ParameterType type = getParameterType(parameter);
    switch (type) { 
      case PARAM_ROUTE_ALLOWED_METH: {
        std::string allowedMethod;
        while (routeIss >> allowedMethod)
          route.allowedMethods.insert(allowedMethod);
        }
        break;
      case PARAM_ROUTE_ROOT:
        routeIss >> route.root;
        break;
      case PARAM_ROUTE_INDEX:
        routeIss >> route.index;
        break;
      case PARAM_ROUTE_RETURN:
        routeIss >> route.redirect;
        break;
      case PARAM_ROUTE_AUTOINDEX:
        routeIss >> route.autoindex;
        break;
      default:
        throw std::runtime_error("Error parsing configuration file");
    }
  }
  server.routes[routePath] = route;
}

}  // namespace

void Config::loadFromFile(const std::string& path) {
  std::ifstream file(path.c_str());

  if (!file.is_open())
    throw std::runtime_error("File at " + path + " does not exist");

  std::string line;
  while (std::getline(file, line)) {
    if (line.at(line.size() - 1) == '{') {
      ConfigTypes::ServerConfig server;

      while (std::getline(file, line) && line.at(line.size() - 1) != '}') {
        std::string parameter;
        std::istringstream iss(line);
        iss >> parameter;
        if (parameter.empty())
          continue;
        ParameterType type = getParameterType(parameter);
        switch (type) {
          case PARAM_LISTEN:
            if (!(iss >> server.port))
              throw std::runtime_error(
                  "Conversion to int error in port attribute");
            break;
          case PARAM_SERVER_NAMES: {
            std::string serverName;
            while (iss >> serverName)
              server.serverNames.insert(serverName);
          } break;
          case PARAM_LOCATION:
            parseRoute(server, iss, file);
            break;
          default:
            throw std::runtime_error("Error parsing configuration file");
        }
      }
      servers.push_back(server);
    }
  }
};

ConfigTypes::ServerConfig& Config::getServerConfig(
    const std::string& serverName) {
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
