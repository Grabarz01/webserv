#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <map>
#include <set>
#include <string>
#include <vector>

/**
 * @namespace ConfigTypes
 * @brief Contains types and structures for configuration.
 */
namespace ConfigTypes {

/**
 * @struct RouteConfig
 * @brief Configuration settings for a specific route.
 *
 * Contains route-specific settings such as the document root, index file,
 * autoindex flag, allowed HTTP methods, allowed CGI extensions, redirection
 * rules, maximum allowed body size, and upload path.
 */
struct RouteConfig {
  std::string root;                      ///< Document root for the route.
  std::string index;                     ///< Default index file.
  std::string autoindex;                 ///< Autoindex setting.
  std::set<std::string> allowedMethods;  ///< Set of permitted HTTP methods.
  std::vector<std::string>
      cgiAllowedExtensions;           ///< List of allowed CGI extensions.
  std::vector<std::string> redirect;  ///< Redirection rules.
  std::string maxBodySize;  ///< Maximum allowed size for the request body.
  std::string uploadPath;   ///< Path for uploaded files.
};

/**
 * @struct ServerConfig
 * @brief Configuration settings for a server.
 *
 * Defines the server-level configuration including host:port pairs,
 * server names, paths for CGI interpreters, custom error pages, the default
 * route, and route-specific configurations.
 */
struct ServerConfig {
  std::set<std::string>
      hostPortPairs;  ///< Set of host and port pairs the server listens on.
  std::set<std::string> serverNames;  ///< Set of server names.
  std::string cgiPathPython;          ///< Path to the Python CGI interpreter.
  std::string cgiPathPhp;             ///< Path to the PHP CGI interpreter.
  std::map<unsigned int, std::string>
      errorPages;  ///< Custom error pages mapped by HTTP status codes.
  RouteConfig defaultRoute;  ///< Default route configuration.
  std::map<std::string, RouteConfig>
      routes;  ///< Map of route configurations indexed by route name/path.
};

}  // namespace ConfigTypes

/**
 * @class Config
 * @brief Loads and stores server configuration.
 *
 * The Config class is responsible for loading configuration data from a file,
 * parsing it into usable structures, and providing access to the configuration
 * settings.
 */
class Config {
 private:
  std::vector<ConfigTypes::ServerConfig>
      servers;  ///< Vector of server configurations.
  std::set<std::string>
      hostPortPairsForConfig;  ///< A set of host:port pairs extracted from the
                               ///< configuration.

  /**
   * @brief Initializes the hostPortPairsForConfig set.
   *
   * Parses and sets the host:port pairs from the loaded server configurations.
   */
  void setHostPortPairsForConfig();

 public:
  /**
   * @brief Loads configuration from a file.
   *
   * Reads and parses the configuration file specified by the provided path.
   *
   * @param path Path to the configuration file.
   */
  void loadFromFile(const std::string& path);

  /**
   * @brief Retrieves a server configuration based on host:port and server name.
   *
   * Searches through the loaded server configurations and returns a reference
   * to the matching configuration.
   *
   * @param hostPortPair The host:port pair identifying the server.
   * @param serverName The server name to match.
   * @return Reference to the corresponding ServerConfig.
   */
  ConfigTypes::ServerConfig& getServerConfig(const std::string& hostPortPair,
                                             const std::string& serverName);

  /**
   * @brief Retrieves the set of host:port pairs from the configuration.
   *
   * @return A constant reference to the set of host:port pairs.
   */
  const std::set<std::string>& getHostPortPairsForConfig() const;
};

#endif