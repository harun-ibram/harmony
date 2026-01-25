#include <harmony/net.hpp>
#include <harmony/protocol.hpp>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstdio>
#include <cstring>
#include <iostream>
#include <string>

int main()
{
    const char *SERVER_IP = "127.0.0.1";
    const int PORT = 7727;
    int ret;

    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0)
    {
        perror("socket");
        return -1;
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);

    ret = inet_pton(AF_INET, SERVER_IP, &addr.sin_addr);
    if (ret != 1)
    {
        std::cerr << "inet_pton failed for IP: " << SERVER_IP << "\n";
        close(fd);
        return -1;
    }

    if (connect(fd, reinterpret_cast<sockaddr *>(&addr), sizeof(addr)) < 0)
    {
        perror("connect");
        close(fd);
        return -1;
    }

    std::cout << "Connected to " << SERVER_IP << ":" << PORT << "\n";

    std::string msg = harmony::proto::make_login("sunny");

    if (!harmony::send_message(fd, msg))
    {
        std::cout << "OH ITS BAD\n";
        perror("send");
        close(fd);
        return -1;
    }

    std::cout << "Sent: " << msg << "\n";

    std::string reply;
    if (!harmony::recv_message(fd, reply))
    {
        perror("recv");
        close(fd);
        return -1;
    }

    std::cout << "Reply: " << reply << "\n";

    close(fd);
    return 0;
}