#include <harmony/net.hpp>
#include <harmony/protocol.hpp>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstdio>
#include <cstdlib>
#include <string>
#include <optional>
#include <iostream>

int main()
{
    const int PORT = 7727;

    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0) {
        perror("socket");
        return -1;
    }

    int yes = 1;
    if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) < 0) {
        perror("setsockopt(SO_REUSEADDR)");
        return -1;  
    }

    int ret;

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    ret = bind(listen_fd, reinterpret_cast<sockaddr *>(&addr), sizeof(addr));

    if (ret < 0) {
        perror("bind");
        return -1;
    }

    ret = listen(listen_fd, 1);
    if (ret < 0) {
        perror("listen");
        return -1;
    }

    std::printf("Server listening on 0.0.0.0:%d\n", PORT);

    sockaddr_in client_addr{};
    socklen_t client_len = sizeof(client_addr);

    int client_fd = accept(listen_fd, reinterpret_cast<sockaddr *>(&client_addr), &client_len);

    if (client_fd < 0) {
        perror("accept");
        close(listen_fd);
        return -1;
    }

    char client_ip_str[INET_ADDRSTRLEN];

    inet_ntop(
        AF_INET,
        &client_addr.sin_addr,
        client_ip_str,
        sizeof(client_ip_str)
    );

    int client_port = ntohs(client_addr.sin_port);

    std::cout << "Accepted connection from " << client_ip_str << ":" << client_port << std::endl;

    // Main loop: receive -> print -> (optional parse) -> reply
    while (true) {
        std::string incoming;

        if (!harmony::recv_message(client_fd, incoming)) {
            break;
        }

        std::cout << "Received message: " << incoming << std::endl;

        auto parsed = harmony::proto::parse(incoming);
        if (!parsed) {
            std::string error = harmony::proto::make_error("Bad message");
            if (!harmony::send_message(client_fd, error)) {
                break;
            }
            continue;
        }

        std::string ack = harmony::proto::make_event("ack");
        if (!harmony::send_message(client_fd, ack)) {
            break;
        }
    }


    close(client_fd);
    close(listen_fd);

    return 0;
}
