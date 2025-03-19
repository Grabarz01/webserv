#ifndef HTTPRESPONSE_HPP
#define HTTPRESPONSE_HPP

#include <map>
#include <string>

/**
 * @class HttpResponse
 * @brief Class for generating HTTP responses.
 *
 * This class allows the configuration, generation, and retrieval of complete
 * HTTP response messages. It supports setting custom error pages and
 * constructing responses based on HTTP status codes, headers, and a response
 * body.
 */
class HttpResponse {
 private:
  unsigned int status;     ///< HTTP status code.
  std::string response;    ///< Complete HTTP response message.
  std::string statusLine;  ///< HTTP status line (e.g. "HTTP/1.1 200 OK").
  std::map<std::string, std::string>
      headers;  ///< Map of HTTP header key-value pairs.
  std::map<unsigned int, std::string>
      errorPages;    ///< Mapping from error codes to custom error page paths.
  std::string body;  ///< HTTP response body.

  /**
   * @brief Sets the HTTP status line.
   *
   * Constructs the status line based on the current status code.
   */
  void setStatusLine();

  /**
   * @brief Sets the HTTP headers.
   *
   * Populates the header fields using the provided response headers.
   *
   * @param responseHeaders A map of additional header key-value pairs.
   */
  void setHeaders(const std::map<std::string, std::string>& responseHeaders);

  /**
   * @brief Generates the body for an error response.
   *
   * Creates the default HTML page for error responses when a custom error page
   * is not provided.
   *
   * @return A string containing the error page content.
   */
  std::string setBodyForError();

 public:
  /**
   * @brief Configures the HTTP response.
   *
   * Sets the response status, custom error pages, and response body.
   *
   * @param responseStatus The HTTP status code.
   * @param errorPages A map associating HTTP status codes with custom error
   * page paths.
   * @param responseBody The response body content.
   */
  void configure(const unsigned int responseStatus,
                 const std::map<unsigned int, std::string>& errorPages,
                 const std::string& responseBody);

  /**
   * @brief Sets the HTTP status code.
   *
   * @param responseStatus The HTTP status code to set.
   */
  void setStatus(const unsigned int responseStatus);

  /**
   * @brief Sets custom error pages.
   *
   * @param errorPages A map associating HTTP error status codes with file paths
   * to custom error pages.
   */
  void setErrorPages(const std::map<unsigned int, std::string>& errorPages);

  /**
   * @brief Sets the response body.
   *
   * @param responseBody The content to be used as the body of the response.
   */
  void setBody(const std::string& responseBody);

  /**
   * @brief Retrieves the complete HTTP response.
   *
   * @return A string containing the full HTTP response.
   */
  std::string getResponse() const;

  /**
   * @brief Generates the full HTTP response.
   *
   * Constructs the HTTP response by setting the status line, headers, and body.
   *
   * @param responseHeaders A map containing additional header key-value pairs.
   */
  void generateResponse(
      const std::map<std::string, std::string>& responseHeaders);
};

#endif