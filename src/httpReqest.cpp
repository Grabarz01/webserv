#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <iostream>
#include <map>
#include <sstream>
#include <string>

typedef struct s_httpRequest {
  std::string method;
  std::string path;
  std::string version;
  std::string host;
  std::string sth;

} t_httpRequest;

void getRequset(std::string buffer) {
  t_httpRequest request;
  std::string temp;
  std::istringstream ask(buffer);
  ask >> request.method >> request.path >> request.version;

  std::string header_line;
  std::getline(ask, header_line);
  while (std::getline(ask, header_line) && header_line != "\r") {
    std::istringstream header_stream(header_line);
    std::string key;
    if (std::getline(header_stream, key, ':')) {
      std::string value;
      std::getline(header_stream, value);
      headers[key] = value.substr(1);  // Usuń spację na początku wartości
    }
  }

  std::cout << temp << std::endl;

  std::cout << "\nNagłówki:\n";
  for (const auto& [key, value] : headers) {
    std::cout << key << ": " << value << "\n";
  }

  if post
    then body....
  // odebranie zapytania i odpowiednie jego sprawdzenie
}

int main() {
  std::string http_request =
      "GET /index.html HTTP/1.1\r\n"
      "Host: www.example.com\r\n"
      "User-Agent: Mozilla/5.0\r\n"
      "Accept: text/html\r\n"
      "\r\n";

  getRequset(http_request);
}