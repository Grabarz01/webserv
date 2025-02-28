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

// TODO: extensive tests
void parseRoute(ConfigTypes::ServerConfig& server,
                std::istringstream& iss,
                std::ifstream& file) {
  ConfigTypes::RouteConfig route;
  std::string routePath;
  iss >> routePath;
  if (routePath == "{" || routePath.empty())
    throw std::runtime_error("Configuration path: incorrect path in route");

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
        while (routeIss >> allowedMethod) {
          if (allowedMethod != "GET" && allowedMethod != "POST" &&
              allowedMethod != "DELETE")
            throw std::runtime_error(
                "Configuration file: incorrect allowed method");
          route.allowedMethods.insert(allowedMethod);
        }
      } break;
      case PARAM_ROUTE_ROOT:
        std::getline(routeIss, route.root);
        break;
      case PARAM_ROUTE_INDEX:
        routeIss >> route.index;
        break;
      case PARAM_ROUTE_RETURN: {
        std::string temp;
        while (routeIss >> temp)
          server.defaultRoute.redirect.push_back(temp);
      } break;
      case PARAM_ROUTE_AUTOINDEX: {
        std::string temp;
        routeIss >> temp;
        if (temp != "off" && temp != "on")
          throw std::runtime_error(
              "Configuration file: incorrect autoindex value: " + temp);
        server.defaultRoute.autoindex = (temp == "on");
      } break;
      case PARAM_CLOSING_BRACKET: {
        server.routes[routePath] = route;
        return;
      } break;
      default:
        throw std::runtime_error(
            "Configuration file: unexpected parameter in route: " + parameter);
    }
  }
  throw std::runtime_error("Configuration file: expected closing bracket");
}

void setDefaultValues(ConfigTypes::ServerConfig& server) {
  if (server.hostPortPairs.empty())
    server.hostPortPairs.insert("0.0.0.0:80");
  if (server.defaultRoute.allowedMethods.empty()) {
    server.defaultRoute.allowedMethods.insert("GET");
    server.defaultRoute.allowedMethods.insert("POST");
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
        std::string temp;
        iss >> temp;
        if (temp.find(':') == std::string::npos)
          throw std::runtime_error(
              "Configuration file: invalid host:port definition: " + temp);
        server.hostPortPairs.insert(temp);
      } break;
      case PARAM_SERVER_NAMES: {
        std::string serverName;
        while (iss >> serverName)
          server.serverNames.insert(serverName);
      } break;
      case PARAM_LOCATION:
        parseRoute(server, iss, file);
        break;
      case PARAM_ROUTE_ALLOWED_METH: {
        std::string allowedMethod;
        while (iss >> allowedMethod) {
          if (allowedMethod != "GET" && allowedMethod != "POST" &&
              allowedMethod != "DELETE")
            throw std::runtime_error(
                "Configuration file: incorrect allowed method");
          server.defaultRoute.allowedMethods.insert(allowedMethod);
        }
      } break;
      case PARAM_ROUTE_ROOT:
        std::getline(iss, server.defaultRoute.root);
        break;
      case PARAM_ROUTE_INDEX:
        iss >> server.defaultRoute.index;
        break;
      case PARAM_ROUTE_RETURN: {
        std::string temp;
        while (iss >> temp)
          server.defaultRoute.redirect.push_back(temp);
      } break;
      case PARAM_ROUTE_AUTOINDEX: {
        std::string temp;
        iss >> temp;
        if (temp != "off" && temp != "on")
          throw std::runtime_error("Configuration file: incorrect autoindex: " +
                                   temp);
        server.defaultRoute.autoindex = (temp == "on");
      } break;
      case PARAM_CLOSING_BRACKET: {
        setDefaultValues(server);
        return;
      } break;
      default:
        throw std::runtime_error(
            "Configuration file: unexpected parameter in server: " + parameter);
    }
  }
  throw std::runtime_error("Configuration file: expected closing bracket");
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
    if (line == "server {") {
      ConfigTypes::ServerConfig server;
      parseServer(server, file);
      servers.push_back(server);
    } else
      throw std::runtime_error("Unexpected line in configuration file");
  }
  setHostPortPairsForConfig();
};

ConfigTypes::ServerConfig& Config::getServerConfig(
    const std::string& portHostPair,
    const std::string& serverName) {
  std::vector<ConfigTypes::ServerConfig>::iterator it = servers.begin();
  for (; it != servers.end(); it++) {
    if (it->hostPortPairs.find(portHostPair) != it->hostPortPairs.end()) {
      if (it->serverNames.find(serverName) != it->serverNames.end())
        return *it;
    }
  }
  // if server with matching portHostPair and serverName not found, try to
  // return first server with matching portHostPair
  for (it = servers.begin(); it != servers.end(); it++) {
    if (it->hostPortPairs.find(portHostPair) != it->hostPortPairs.end())
      return *it;
  }
  throw std::runtime_error("No configuration found for host:port " +
                           portHostPair);
}

void Config::setHostPortPairsForConfig() {
  std::vector<ConfigTypes::ServerConfig>::iterator it = servers.begin();
  for (; it != servers.end(); it++) {
    std::set<std::string>::iterator it2 = it->hostPortPairs.begin();
    for (; it2 != it->hostPortPairs.end(); it2++) {
      hostPortPairsForConfig.insert(*it2);
    }
  }
}

const std::set<std::string>& Config::getHostPortPairsForConfig() const {
  return hostPortPairsForConfig;
}