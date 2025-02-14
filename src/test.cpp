#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#define PORT 8080
#define BUFFER_SIZE 4096

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    char buffer[BUFFER_SIZE] = {0};
    
    // Tworzenie gniazda
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    // Ustawienia adresu serwera
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Bindowanie (przypisanie adresu i portu)
    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    // Nasłuchiwanie (maksymalnie 3 połączenia w kolejce)
    if (listen(server_fd, 3) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    std::cout << "Serwer nasłuchuje na porcie " << PORT << "..." << std::endl;

    while (true) {
        socklen_t addrlen = sizeof(address);
        new_socket = accept(server_fd, (struct sockaddr*)&address, &addrlen);
        if (new_socket < 0) {
            perror("Accept failed");
            exit(EXIT_FAILURE);
        }

        // Odczyt zapytania HTTP
        read(new_socket, buffer, BUFFER_SIZE);
        std::cout << "Otrzymano zapytanie:\n" << buffer << std::endl;

        // Prosta odpowiedź HTTP
        std::string http_response = 
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/html\r\n"
            "Content-Length: 39\r\n"
            "\r\n"
            "<html><body><h1>Witaj!</h1></body></html>";

        send(new_socket, http_response.c_str(), http_response.length(), 0);
        close(new_socket); // Zamknięcie połączenia
    }

    return 0;
}
