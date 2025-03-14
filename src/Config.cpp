#include "Config.hpp"

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>

namespace {
enum ParameterType {
  PARAM_LISTEN,
  PARAM_SERVER_NAMES,
  PARAM_LOCATION,
  PARAM_CGIPATH_PYTHON,
  PARAM_CGIPATH_PHP,
  PARAM_ERROR_PAGES,
  PARAM_UNKNOWN,
  PARAM_ROUTE_ALLOWED_METH,
  PARAM_ROUTE_ROOT,
  PARAM_ROUTE_INDEX,
  PARAM_ROUTE_RETURN,
  PARAM_ROUTE_AUTOINDEX,
  PARAM_ROUTE_CGIALLOWEDEXT,
  PARAM_ROUTE_MAX_BODY_SIZE,
  PARAM_ROUTE_UPLOAD_PATH,
  PARAM_CLOSING_BRACKET
};

ParameterType getParameterType(const std::string& param) {
  if (param == "listen")
    return PARAM_LISTEN;
  else if (param == "serverNames")
    return PARAM_SERVER_NAMES;
  else if (param == "location")
    return PARAM_LOCATION;
  else if (param == "cgiPathPython")
    return PARAM_CGIPATH_PYTHON;
  else if (param == "cgiPathPhp")
    return PARAM_CGIPATH_PHP;
  else if (param == "errorPage")
    return PARAM_ERROR_PAGES;
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
  else if (param == "cgiAllowedExtensions")
    return PARAM_ROUTE_CGIALLOWEDEXT;
  else if (param == "maxBodySize")
    return PARAM_ROUTE_MAX_BODY_SIZE;
  else if (param == "uploadPath")
    return PARAM_ROUTE_UPLOAD_PATH;
  else if (param == "}")
    return PARAM_CLOSING_BRACKET;
  else
    return PARAM_UNKNOWN;
}

void setDefaultValues(ConfigTypes::ServerConfig& server) {
  if (server.hostPortPairs.empty())
    server.hostPortPairs.insert("0.0.0.0:8080");
  if (server.defaultRoute.allowedMethods.empty()) {
    server.defaultRoute.allowedMethods.insert("GET");
    server.defaultRoute.allowedMethods.insert("POST");
  }
  if (server.defaultRoute.maxBodySize.empty())
    server.defaultRoute.maxBodySize = "1024";
  if (server.defaultRoute.autoindex.empty())
    server.defaultRoute.autoindex = "off";
}

void setRouteParameterValue(ParameterType type,
                            std::istringstream& iss,
                            ConfigTypes::RouteConfig& route) {
  switch (type) {
    case PARAM_ROUTE_ALLOWED_METH: {
      std::string allowedMethod;
      while (iss >> allowedMethod) {
        if (allowedMethod != "GET" && allowedMethod != "POST" &&
            allowedMethod != "DELETE")
          throw std::runtime_error(
              "Configuration file: incorrect allowed method");
        route.allowedMethods.insert(allowedMethod);
      }
    } break;
    case PARAM_ROUTE_ROOT:
      iss >> route.root;
      break;
    case PARAM_ROUTE_INDEX:
      iss >> route.index;
      break;
    case PARAM_ROUTE_RETURN: {
      std::string temp;
      while (iss >> temp)
        route.redirect.push_back(temp);
      if (route.redirect.size() != 2)
        throw std::runtime_error("Configuration file: invalid redirect" + temp);
    } break;
    case PARAM_ROUTE_AUTOINDEX: {
      std::string temp;
      iss >> temp;
      if (temp != "off" && temp != "on")
        throw std::runtime_error(
            "Configuration file: incorrect autoindex value: " + temp);
      route.autoindex = temp;
    } break;
    case PARAM_ROUTE_CGIALLOWEDEXT: {
      std::string temp;
      while (iss >> temp) {
        if (temp != ".py" && temp != ".php")
          throw std::runtime_error("Configuration file: cgi extension " + temp +
                                   " not allowed");
        route.cgiAllowedExtensions.push_back(temp);
      }
    } break;
    case PARAM_ROUTE_MAX_BODY_SIZE: {
      iss >> route.maxBodySize;
      for (size_t i; i < route.maxBodySize.size(); i++) {
        if (!isdigit(route.maxBodySize[i]))
          throw std::runtime_error("Configuration file: maxBodySize invalid");
      }
    } break;
    case PARAM_ROUTE_UPLOAD_PATH:
      iss >> route.uploadPath;
      break;
    default:
      throw std::runtime_error(
          "Configuration file: unexpected parameter in route");
  }
}

void parseDefaultRoute(ConfigTypes::ServerConfig& server,
                       std::istringstream& iss,
                       ParameterType type) {
  ConfigTypes::RouteConfig& route = server.defaultRoute;
  setRouteParameterValue(type, iss, route);
}

void parseLocationRoute(ConfigTypes::ServerConfig& server,
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
    if (parameter.empty() || parameter.at(0) == '#')
      continue;

    ParameterType type = getParameterType(parameter);
    if (type == PARAM_CLOSING_BRACKET)
      break;
    setRouteParameterValue(type, routeIss, route);
  }
  server.routes[routePath] = route;
}

void parseServer(ConfigTypes::ServerConfig& server, std::ifstream& file) {
  std::string line;
  while (std::getline(file, line)) {
    if (line.empty())
      continue;

    std::string parameter;
    std::istringstream iss(line);
    iss >> parameter;

    if (parameter.empty() || parameter.at(0) == '#')
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
      case PARAM_LOCATION:
        parseLocationRoute(server, iss, file);
        break;
      case PARAM_SERVER_NAMES: {
        std::string serverName;
        while (iss >> serverName)
          server.serverNames.insert(serverName);
      } break;
      case PARAM_CGIPATH_PYTHON:
        iss >> server.cgiPathPython;
        break;
      case PARAM_CGIPATH_PHP:
        iss >> server.cgiPathPhp;
        break;
      case PARAM_ERROR_PAGES: {
        unsigned int errCode;
        std::string path;
        if (!(iss >> errCode))
          throw std::runtime_error(
              "Configuration file: Expected error code number in 'errorPages'");
        iss >> path;
        server.errorPages[errCode] = path;
      } break;
      case PARAM_CLOSING_BRACKET: {
        setDefaultValues(server);
        return;
      } break;
      default:
        parseDefaultRoute(server, iss, type);
        break;
    }
  }
  throw std::runtime_error("Configuration file: expected closing bracket");
}

}  // namespace

void Config::loadFromFile(const std::string& path) {
  std::ifstream file(path.c_str());
  if (!file.is_open())
    throw std::runtime_error("File at " + path + " does not exist");
  if (file.peek() == std::ifstream::traits_type::eof()) {
    throw std::runtime_error("File at " + path + " is empty");
  }

  std::string line;
  while (std::getline(file, line)) {
    if (line.empty())
      continue;
    if (line == "server {") {
      ConfigTypes::ServerConfig server;
      parseServer(server, file);
      servers.push_back(server);
    } else
      throw std::runtime_error("Invalid configuration file");
  }
  setHostPortPairsForConfig();
};

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

ConfigTypes::ServerConfig& Config::getServerConfig(
  const std::string& HostPortPair,
  const std::string& serverName) {
std::vector<ConfigTypes::ServerConfig>::iterator it = servers.begin();
for (; it != servers.end(); it++) {
  if (it->hostPortPairs.find(HostPortPair) != it->hostPortPairs.end()) {
    if (it->serverNames.find(serverName) != it->serverNames.end())
      return *it;
  }
}
// if server with matching HostPortPair and serverName not found, try to
// return first server with matching HostPortPair
for (it = servers.begin(); it != servers.end(); it++) {
  if (it->hostPortPairs.find(HostPortPair) != it->hostPortPairs.end()) {
    std::cout << "No serverName " << serverName << " found for "
              << HostPortPair << ". Returned first matching configuration."
              << std::endl;
    return *it;
  }
}
throw std::runtime_error("No configuration found for host:port " +
                         HostPortPair);
}