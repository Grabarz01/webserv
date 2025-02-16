#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <poll.h>
#include <sys/socket.h>
#include <unistd.h>
#include <algorithm>
#include <cstring>
#include <iostream>
#include <vector>

#define PORT 8080
#define BUFFER_SIZE 4096

void set_nonblocking(int socket) {
  int flags = fcntl(socket, F_GETFL, 0);
  fcntl(socket, F_SETFL, flags | O_NONBLOCK);
}

int main() {
  int ports_arr[] = {8080, 9090};
  std::vector<int> ports(ports_arr,
                         ports_arr + sizeof(ports_arr) / sizeof(int));
  std::vector<pollfd> fds;

  std::vector<int>::iterator it;
  for (it = ports.begin(); it != ports.end(); ++it) {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
      perror("Socket failed");
      return 1;
    }

    sockaddr_in address;
    std::memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    inet_pton(AF_INET, "127.0.0.1", &address.sin_addr);
    address.sin_port = htons(*it);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
      perror("Bind failed");
      return 1;
    }

    if (listen(server_fd, 10) < 0) {
      perror("Listen failed");
      return 1;
    }

    set_nonblocking(server_fd);
    pollfd pfd = {server_fd, POLLIN, 0};
    fds.push_back(pfd);
  }

  std::cout << "Listens on ports: \n";
  for (it = ports.begin(); it != ports.end(); ++it) {
    std::cout << *it << " " << std::endl;
  }

  while (true) {
    if (poll(&fds[0], fds.size(), -1) < 0) {
      perror("poll error");
      return 1;
    }

    std::vector<pollfd>::iterator fd_it = fds.begin();
    while (fd_it != fds.end()) {
      if (fd_it->revents & POLLIN) {
        if (std::distance(fds.begin(), fd_it) < ports.size()) {
          // Nowe połączenie
          int client_fd = accept(fd_it->fd, NULL, NULL);
          if (client_fd >= 0) {
            set_nonblocking(client_fd);
            pollfd client_pfd = {client_fd, POLLIN, 0};
            fds.push_back(client_pfd);
            std::cout << "Nowe połączenie na porcie "
                      << *ports.begin() + std::distance(fds.begin(), fd_it)
                      << "\n";
          }
        } else {
          // Dane od klienta - do poprawy
          char buffer[4024];
          std::memset(buffer, 0, sizeof(buffer));
          // odczytanie wiadomosci
          recv(fd_it->fd, buffer, sizeof(buffer), 0);
          const char* response =
              "HTTP/1.1 200 OK\r\nContent-Length: 13\r\n\r\nHello, world!";
          int response_len = std::strlen(response);
          std::cout << "Odebrano: " << buffer << "\n";
          send(fd_it->fd, response, response_len, 0);
        }
      }
      ++fd_it;
    }
  }
  return 0;
}
