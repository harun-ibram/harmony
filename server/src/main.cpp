#include <harmony/net.hpp>
#include <harmony/protocol.hpp>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <poll.h>

#include <cstdio>
#include <cstdlib>
#include <string>
#include <optional>
#include <vector>
#include <iostream>
#include <cstdint>
#include <cstring>

struct Client {
    int fd = -1;
    std::vector<uint8_t> in;
};

int main()
{
    const int PORT = 7727;

    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0)
    {
        perror("socket");
        return -1;
    }

    int yes = 1;
    if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) < 0)
    {
        perror("setsockopt(SO_REUSEADDR)");
        return -1;
    }

    int ret;

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    ret = bind(listen_fd, reinterpret_cast<sockaddr *>(&addr), sizeof(addr));

    if (ret < 0)
    {
        perror("bind");
        return -1;
    }

    ret = listen(listen_fd, 1);
    if (ret < 0)
    {
        perror("listen");
        return -1;
    }

    std::printf("Server listening on 0.0.0.0:%d\n", PORT);

    // Prepare for polling
    std::vector<pollfd> fds;
    std::vector<Client> clients;

    pollfd poll_listen;
    poll_listen.fd = listen_fd;
    poll_listen.events = POLLIN;
    poll_listen.revents = 0;

    fds.push_back(poll_listen);

    // Crazy stuff from here!
    while (true)
    {
        int ready = poll(fds.data(), fds.size(), -1);

        if (ready < 0)
        {
            if (errno == EINTR)
                continue;
            perror("poll");
            break;
        }

        // New connection
        if (fds[0].revents & POLLIN)
        {
            int client_fd = accept(listen_fd, nullptr, nullptr);
            if (client_fd >= 0)
            {
                fds.push_back({client_fd, POLLIN, 0});
                clients.push_back({client_fd, {}});
            }
            else if (errno != EINTR)
            {
                perror("accept");
            }
        }


        for (size_t i = 1; i < fds.size(); /* increment inside */)
        {
            // If client hung up / error, drop them
            if (fds[i].revents & (POLLHUP | POLLERR | POLLNVAL))
            {
                close(fds[i].fd);
                fds.erase(fds.begin() + i);
                clients.erase(clients.begin() + (i - 1));
                continue;
            }

            if (fds[i].revents & POLLIN)
            {
                char buf[4096];
                ssize_t n = recv(fds[i].fd, buf, sizeof(buf), 0);

                if (n == 0)
                {
                    // client disconnected cleanly
                    close(fds[i].fd);
                    fds.erase(fds.begin() + i);
                    clients.erase(clients.begin() + (i - 1));
                    continue;
                }

                if (n < 0)
                {
                    if (errno == EINTR)
                    {
                        i++;
                        continue;
                    }
                    // If you made sockets non-blocking, also ignore EAGAIN/EWOULDBLOCK here.
                    perror("recv");
                    close(fds[i].fd);
                    fds.erase(fds.begin() + i);
                    clients.erase(clients.begin() + (i - 1));
                    continue;
                }

                Client &client = clients[i - 1];
                client.in.insert(client.in.end(),
                                 reinterpret_cast<uint8_t *>(buf),
                                 reinterpret_cast<uint8_t *>(buf) + n);

                while (client.in.size() >= sizeof(uint32_t))
                {
                    uint32_t msg_len = 0;
                    std::memcpy(&msg_len, client.in.data(), sizeof(uint32_t));

                    if (client.in.size() < sizeof(uint32_t) + msg_len)
                    {
                        break;
                    }

                    std::string msg(
                        client.in.begin() + sizeof(uint32_t),
                        client.in.begin() + sizeof(uint32_t) + msg_len);

                    client.in.erase(
                        client.in.begin(),
                        client.in.begin() + sizeof(uint32_t) + msg_len);

                    // Log messages that are being sent
                    std::cout << "Broadcasting: " << msg << std::endl;

                    // Broadcast to everyone else (excluding sender)
                    for (size_t j = 1; j < fds.size(); /* increment inside */)
                    {
                        if (j == i)
                        {
                            j++;
                            continue;
                        }

                        uint32_t out_len = static_cast<uint32_t>(msg.size());
                        size_t sent_total = 0;
                        while (sent_total < sizeof(out_len))
                        {
                            ssize_t s = send(
                                fds[j].fd,
                                reinterpret_cast<const char *>(&out_len) + sent_total,
                                sizeof(out_len) - sent_total,
                                0);
                            if (s <= 0)
                                break;
                            sent_total += static_cast<size_t>(s);
                        }

                        if (sent_total < sizeof(out_len))
                        {
                            close(fds[j].fd);
                            fds.erase(fds.begin() + j);
                            clients.erase(clients.begin() + (j - 1));

                            if (j < i)
                                i--;

                            continue;
                        }

                        sent_total = 0;
                        while (sent_total < msg.size())
                        {
                            ssize_t s = send(
                                fds[j].fd,
                                msg.data() + sent_total,
                                msg.size() - sent_total,
                                0);
                            if (s <= 0)
                                break;
                            sent_total += static_cast<size_t>(s);
                        }

                        if (sent_total < msg.size())
                        {
                            close(fds[j].fd);
                            fds.erase(fds.begin() + j);
                            clients.erase(clients.begin() + (j - 1));

                            if (j < i)
                                i--;

                            continue;
                        }

                        j++;
                    }
                }
            }

            i++;
        }
    }

    close(listen_fd);

    return 0;
}
