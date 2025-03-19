#ifndef CGI_HPP
#define CGI_HPP

#include <map>
#include <string>
#include <vector>

/**
 * @enum cgiExt
 * @brief Enumeration for supported CGI script extensions.
 */
enum cgiExt {
  CGI_PHP,    ///< PHP CGI extension.
  CGI_PYTHON  ///< Python CGI extension.
};

/**
 * @class Cgi
 * @brief Class to handle CGI execution.
 *
 * This class encapsulates functionality to prepare the environment for CGI
 * script execution, run the CGI script, and retrieve the response generated
 * by the script.
 */
class Cgi {
 private:
  /**
   * @brief Extracts the query string from the provided path.
   *
   * @param path Reference to the URL or path string.
   * @return A string containing the query portion of the URL.
   */
  std::string extractQuery(std::string& path);

  /**
   * @brief Extracts the path info from the provided path.
   *
   * This method extracts the additional path information based on the specified
   * size.
   *
   * @param path Reference to the complete URL or path string.
   * @param size Number of characters to consider for extraction.
   * @return A string containing the path info.
   */
  std::string extractPathInfo(std::string& path, size_t size);

  /**
   * @brief Extracts the CGI script path from the provided path.
   *
   * @param path Reference to the complete URL or path string.
   * @param size Number of characters to consider for extracting the script
   * path.
   * @return A string containing the script path.
   */
  std::string extractScriptPath(std::string& path, size_t size);

  std::string queryString;  ///< Query string extracted from the URL.
  std::string pathInfo;     ///< Additional path information.
  std::string scriptPath;   ///< Filesystem path to the CGI script.
  char* cgiEnvp[7];     ///< Array of environment variables for the CGI process.
  char* cgiPath;        ///< Path to the CGI interpreter.
  char* cgiScriptPath;  ///< Path to the CGI script to run.
  size_t posPython;     ///< Position index for Python CGI.
  size_t posPhp;        ///< Position index for PHP CGI.
  size_t posQuery;      ///< Position index for query extraction.
  size_t posCgiExt;     ///< Position index for CGI extension.
  cgiExt cgiExtension;  ///< The current CGI extension in use.

 public:
  /**
   * @brief Destructor for the Cgi class.
   */
  ~Cgi();

  /**
   * @brief Default constructor for the Cgi class.
   */
  Cgi();

  /**
   * @brief Checks if the provided path corresponds to a valid CGI request.
   *
   * This method verifies the path with root, CGI interpreter paths, and
   * accepted file extensions.
   *
   * @param pathWithRoot The full filesystem path including document root.
   * @param cgiPathPython Path to the Python CGI interpreter.
   * @param cgiPathPhp Path to the PHP CGI interpreter.
   * @param ext A vector of accepted file extension strings.
   * @return An integer status value indicating success or failure.
   */
  int checkCgi(std::string pathWithRoot,
               std::string cgiPathPython,
               std::string cgiPathPhp,
               std::vector<std::string> ext);

  /**
   * @brief Executes the CGI script.
   *
   * This method runs the CGI script using the prepared environment.
   */
  void runCgi();

  /**
   * @brief Retrieves the path to the CGI script.
   *
   * @return A pointer to a character array representing the CGI script path.
   */
  char* getScriptPath();

  /**
   * @brief Extracts environment variables from the provided path.
   *
   * @param path Reference to the URL or path string from which to extract
   * environment variables.
   */
  void extractEnvFromPath(std::string& path);

  /**
   * @brief Retrieves the query string.
   *
   * @return A string containing the query portion of the URL.
   */
  std::string getQuery(void);

  /**
   * @brief Retrieves the path info.
   *
   * @return A string containing the path info.
   */
  std::string getPath(void);

  /**
   * @brief Retrieves the CGI script identifier.
   *
   * @return A string containing the script identifier.
   */
  std::string getScript(void);

  /**
   * @brief Retrieves the path to the CGI interpreter.
   *
   * @return A pointer to a character array representing the CGI interpreter
   * path.
   */
  char* getCgiPath(void);

  /**
   * @brief Retrieves the environment variables array for the CGI process.
   *
   * @return A pointer to an array of character pointers representing the CGI
   * environment.
   */
  char** getCgiEnvp(void);

  /**
   * @brief Reads the response from the CGI script.
   *
   * Reads the CGI response from a pipe, and extracts the response content and
   * status.
   *
   * @param responseContent Reference to a string to store the response content.
   * @param responseStatus Reference to an unsigned int to store the response
   * status.
   * @param pipe_response Pointer to the pipe file descriptor for reading the
   * response.
   */
  void readResponse(std::string& responseContent,
                    unsigned int& responseStatus,
                    int* pipe_response);

  /**
   * @brief Sets the paths for the CGI interpreters.
   *
   * @param cgiPathPython Path to the Python CGI interpreter.
   * @param cgiPathPhp Path to the PHP CGI interpreter.
   */
  void setCgiPath(std::string cgiPathPython, std::string cgiPathPhp);

  /**
   * @brief Sets the path for the CGI script.
   *
   * Configures the internal CGI script path based on previously determined
   * values.
   */
  void setCgiScriptPath();

  /**
   * @brief Sets the CGI environment variables.
   *
   * Prepares the environment variables for the CGI script execution based on
   * the HTTP method and headers provided.
   *
   * @param method Reference to the HTTP method used for the request.
   * @param headers Map of HTTP request headers.
   */
  void setEnvp(std::string& method,
               std::map<std::string, std::string>& headers);
};

#endif