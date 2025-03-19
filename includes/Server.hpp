#ifndef SERVER_HPP
#define SERVER_HPP

#include <poll.h>
#include <map>
#include <set>
#include <string>
#include <vector>
#include "Config.hpp"

/**
 * @struct ioSocketData
 * @brief Holds information about a client's socket connection.
 *
 * This structure contains the client's host:port identifier and the accumulated
 * raw request data received from that client.
 */
struct ioSocketData {
  std::string hostPortPair;  ///< Client's host:port identifier.
  std::string
      clientRequest;  ///< Accumulated raw HTTP request data from the client.
};

/**
 * @class Server
 * @brief Implements an HTTP server using non-blocking I/O and poll().
 *
 * The Server class handles the initialization, configuration, and management of
 * network sockets. It listens for incoming connections, receives data from
 * clients, generates responses, and sends them back.
 */
class Server {
 private:
  static volatile bool
      running;  ///< Flag that indicates if the server is running.

  /**
   * @brief Signal handler for graceful shutdown.
   *
   * Handles termination signals by setting the running flag to false.
   *
   * @param signum The signal number received.
   */
  static void signalHandler(int signum);

  /**
   * @brief Cleans up resources before server shutdown.
   *
   * Closes open sockets and clears internal data structures.
   */
  void cleanup();

  Config& serversConfig;  ///< Reference to the server configuration object.
  std::vector<std::string>
      hostPortPairs;  ///< List of host:port pairs the server is bound to.
  std::vector<std::string> serverNames;  ///< List of server names.
  std::vector<pollfd>
      pollFds;  ///< Vector of pollfd structures for managing I/O.
  std::map<int, ioSocketData>
      clientFdToIoSocketData;  ///< Mapping from client socket fds to their I/O
                               ///< data.

  /**
   * @brief Sets host and port pairs for the server.
   *
   * Initializes the hostPortPairs vector using a set of host:port strings.
   *
   * @param hostPortPairs A set of host:port pair strings.
   */
  void setHostPortPairs(const std::set<std::string>& hostPortPairs);

  /**
   * @brief Creates a pollfd for a listening socket.
   *
   * Configures a pollfd structure for a listening socket associated with the
   * given host:port pair.
   *
   * @param listenFd The pollfd structure representing the listening socket.
   * @param hostPortPair The host:port pair identifier.
   */
  void createPollFd(const pollfd listenFd, std::string& hostPortPair);

  /**
   * @brief Creates I/O data for a new client connection.
   *
   * Initializes the ioSocketData structure for a new client socket.
   *
   * @param clientFd The file descriptor for the client socket.
   * @param hostPortPair The host:port pair identifier for the client.
   */
  void createIoSocketData(int clientFd, std::string& hostPortPair);

  /**
   * @brief Receives data from a client socket.
   *
   * Reads data from the client socket and appends it to the client's
   * accumulated request.
   *
   * @param pollFdIt Iterator to the pollfd entry corresponding to the client.
   * @return Iterator updated after processing the client data.
   */
  std::vector<pollfd>::iterator receiveDataFromClient(
      std::vector<pollfd>::iterator pollFdIt);

  /**
   * @brief Sends response data to a client.
   *
   * Writes the generated HTTP response to the client socket.
   *
   * @param pollFdIt Iterator to the pollfd entry corresponding to the client.
   * @param response The HTTP response to send.
   * @return Iterator updated after sending the data.
   */
  std::vector<pollfd>::iterator sendDataToClient(
      std::vector<pollfd>::iterator pollFdIt,
      std::string& response);

  /**
   * @brief Generates an HTTP response based on the client's request.
   *
   * Processes the client's request data and constructs an appropriate HTTP
   * response.
   *
   * @param pollFdIt Iterator to the pollfd entry corresponding to the client.
   * @param response Reference to a string where the generated HTTP response is
   * stored.
   */
  void generateResponse(std::vector<pollfd>::iterator pollFdIt,
                        std::string& response);

  /**
   * @brief Retrieves a header value from the client's request.
   *
   * Searches the client request string for the specified header and extracts
   * its value.
   *
   * @param clientRequest The complete raw request string from the client.
   * @param headerParameter The header parameter to search for.
   * @param endChars The characters that delimit the header value.
   * @return The extracted header value, or an empty string if not found.
   */
  std::string getHeaderValue(const std::string& clientRequest,
                             const std::string& headerParameter,
                             const std::string& endChars);

 public:
  /**
   * @brief Constructs a new Server object.
   *
   * Initializes the server using the provided configuration.
   *
   * @param serversConfig Reference to a Config object with server settings.
   */
  Server(Config& serversConfig);

  /**
   * @brief Sets the file descriptor to non-blocking mode.
   *
   * Configures the given socket to operate in non-blocking mode.
   *
   * @param socket The file descriptor of the socket.
   */
  static void setNonblocking(int socket);

  /**
   * @brief Configures server settings and initializes listening sockets.
   */
  void setServer(void);

  /**
   * @brief Adds a file descriptor to the set of monitored sockets.
   *
   * @param fd A pollfd structure representing the file descriptor to add.
   */
  void addFD(pollfd fd);

  /**
   * @brief Main server loop to listen and process client connections.
   *
   * Uses poll() to monitor the sockets and processes incoming connections and
   * data.
   */
  void serverListen();
};

#endif