#include "Config.hpp"

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
  PARAM_ROUTE_AUTOINDEX,
  PARAM_CLOSING_BRACKET
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
  else if (param == "}")
    return PARAM_CLOSING_BRACKET;
  else
    return PARAM_UNKNOWN;
}

// TODO: extensive tests, proper validation
void parseRoute(ConfigTypes::ServerConfig& server,
                std::istringstream& iss,
                std::ifstream& file) {
  ConfigTypes::RouteConfig route;
  std::string routePath;
  iss >> routePath;

  std::string line;
  while (std::getline(file, line)) {
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
      } break;
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
      case PARAM_CLOSING_BRACKET: {
        server.routes[routePath] = route;
        // std::cout << "parsing route done" << std::endl;
        return;
      } break;
      default:
        throw std::runtime_error("Error parsing configuration file");
    }
  }
}

void parseServer(ConfigTypes::ServerConfig& server, std::ifstream& file) {
  std::string line;
  while (std::getline(file, line)) {
    if (line.empty())
      continue;

    std::string parameter;
    std::istringstream iss(line);
    iss >> parameter;

    if (parameter.empty())
      continue;
    ParameterType type = getParameterType(parameter);
    switch (type) {
      case PARAM_LISTEN: {
        unsigned int port;
        while (iss >> port)
          server.ports.insert(port);
      } break;
      case PARAM_SERVER_NAMES: {
        std::string serverName;
        while (iss >> serverName)
          server.serverNames.insert(serverName);
      } break;
      case PARAM_LOCATION:
        parseRoute(server, iss, file);
        break;
      case PARAM_CLOSING_BRACKET:
        return;
        break;
      default:
        throw std::runtime_error("Error parsing configuration file");
    }
  }
}

}  // namespace

void Config::loadFromFile(const std::string& path) {
  std::ifstream file(path.c_str());
  if (!file.is_open())
    throw std::runtime_error("File at " + path + " does not exist");

  std::string line;
  while (std::getline(file, line)) {
    if (line.empty())
      continue;
    if (line.at(line.size() - 1) == '{') {
      ConfigTypes::ServerConfig server;
      parseServer(server, file);
      servers.push_back(server);
    }
    // std::cout << "server parsing done" << std::endl;
  }
  setPorts();
};

ConfigTypes::ServerConfig& Config::getServerConfig(
    const std::string& serverName,
    const std::string& port) {
  unsigned int portInt = std::atoi(port.c_str());
  std::vector<ConfigTypes::ServerConfig>::iterator it = servers.begin();
  for (; it != servers.end(); it++) {
    if (it->ports.find(portInt) != it->ports.end() &&
        it->serverNames.find(serverName) != it->serverNames.end()) {
      return *it;
    }
  }
  throw std::runtime_error("No configuration found for the specified server");
}

void Config::setPorts() {
  std::vector<ConfigTypes::ServerConfig>::iterator it = servers.begin();
  for (; it != servers.end(); it++) {
    std::set<unsigned int>::iterator it2 = it->ports.begin();
    for (; it2 != it->ports.end(); it2++) {
      ports.insert(*it2);
    }
  }
}

const std::set<unsigned int>& Config::getPorts() const {
  return ports;
}