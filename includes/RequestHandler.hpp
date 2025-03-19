#ifndef REQUESTHANDLER_HPP
#define REQUESTHANDLER_HPP

#include <map>
#include <string>
#include "Cgi.hpp"
#include "Config.hpp"

/**
 * @class RequestHandler
 * @brief Parses and handles HTTP requests.
 *
 * The RequestHandler class is responsible for parsing the raw HTTP request,
 * determining the correct route and method, handling CGI where necessary,
 * and generating a proper HTTP response.
 */
class RequestHandler {
 private:
  std::string hostPortPair;  ///< The host and port of the incoming request.
  std::string rawRequest;  ///< The raw request string received from the client.
  std::string route;       ///< The requested route.
  ConfigTypes::RouteConfig
      routeConfig;           ///< Configuration for the current route.
  std::string pathWithRoot;  ///< The full filesystem path after applying the
                             ///< route's root.
  std::string method;        ///< The HTTP method (e.g., GET, POST, DELETE).
  std::string path;          ///< The requested resource path.
  std::string version;       ///< The HTTP version.
  std::map<std::string, std::string> headers;  ///< Parsed HTTP request headers.
  std::string body;           ///< The body of the HTTP request.
  std::string cgiPathPython;  ///< The path to the Python CGI interpreter.
  std::string cgiPathPhp;     ///< The path to the PHP CGI interpreter.
  const std::vector<std::string>
      implementedMethods;  ///< List of implemented HTTP methods.

  unsigned int responseStatus;  ///< HTTP response status code.
  std::string responseContent;  ///< HTTP response body/content.
  std::map<std::string, std::string>
      responseHeaders;  ///< HTTP response headers.
  long conLen;          ///< Content-Length value from the request.

  // Private methods

  /**
   * @brief Parses the raw HTTP request.
   *
   * This method extracts the method, path, version, headers, and body from
   * the raw request data.
   */
  void parseRequest();

  /**
   * @brief Sets the route configuration based on a server configuration.
   *
   * @param serverConfig The server configuration used to determine settings
   *                     for the route.
   */
  void setRouteConfig(const ConfigTypes::ServerConfig& serverConfig);

  /**
   * @brief Handles an HTTP GET request.
   */
  void getReq(void);

  /**
   * @brief Handles an HTTP POST request.
   */
  void postReq(void);

  /**
   * @brief Handles an HTTP DELETE request.
   */
  void deleteReq(void);

  /**
   * @brief Processes a CGI request using the given CGI handler.
   *
   * @param cgi The CGI object responsible for handling the request.
   */
  void getCgiHandler(Cgi& cgi);

  /**
   * @brief Sets the full path with root based on the request route.
   */
  void setPathWithRoot(void);

  /**
   * @brief Processes a redirection if configured.
   */
  void redirect(void);

  /**
   * @brief Checks for the existence of an index file in the requested
   * directory.
   */
  void indexCheck(void);

  /**
   * @brief Handles file uploads.
   */
  void uploadfile(void);

 public:
  /**
   * @brief Constructs a new RequestHandler object.
   *
   * @param rawRequest The raw HTTP request string.
   */
  RequestHandler(std::string rawRequest);

  /**
   * @brief Retrieves the response headers to be sent to the client.
   *
   * @return A constant reference to the map of response headers.
   */
  const std::map<std::string, std::string>& getResponseHeaders(void);

  /**
   * @brief Handles the HTTP request.
   *
   * Processes the request based on the server configuration, invokes the
   * appropriate request method handler, and prepares the response.
   *
   * @param server The server configuration for handling the request.
   */
  void handleRequest(ConfigTypes::ServerConfig& server);

  /**
   * @brief Prints the raw HTTP request to standard output.
   */
  void printRequest();

  /**
   * @brief Generates an autoindex listing for a directory.
   */
  void autoIndex();

  /**
   * @brief Sets the paths for CGI handling.
   *
   * @param php The path to the PHP CGI interpreter.
   * @param python The path to the Python CGI interpreter.
   */
  void setCgiPath(std::string php, std::string python);

  /**
   * @brief Retrieves the raw HTTP request.
   *
   * @return A constant reference to the raw request string.
   */
  const std::string& getRawRequest() const;

  /**
   * @brief Retrieves the response content (body) for the HTTP response.
   *
   * @return A constant reference to the response content string.
   */
  const std::string& getResponseContent() const;

  /**
   * @brief Retrieves the HTTP response status code.
   *
   * @return The HTTP response status code.
   */
  const unsigned int getResponseStatus() const;
};

#endif