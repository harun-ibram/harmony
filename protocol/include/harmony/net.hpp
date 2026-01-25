#pragma once

#include <string>
#include <sys/socket.h>
#include <unistd.h>

namespace harmony {


    /*
    * Send a string over a TCP socket using this format:
    *
    * [4 bytes length][message bytes]
    *
    * Length is the number of bytes in the message.
    */
    inline bool send_message(int socket_fd, const std::string& message) {

        // Send length
        u_int32_t length = message.size();

        if (send(socket_fd, &length, sizeof(length), 0) <= 0) {
            return false;
        }

        // Send actual message
        size_t sent_size = 0;
        while (sent_size < message.size()) {
            ssize_t sent = send(
                socket_fd,
                message.data() + sent_size,
                message.size() - sent_size,
                0
            );

            if (sent <= 0) {
                return false;
            }

            sent_size += sent;
        }

        return true;
    }


    /*
    * Receive a string over a TCP socket using this format:
    *
    * [4 bytes length][message bytes]
    *
    * Length is the number of bytes in the message.
    */
    inline bool recv_message(int socket_fd, std::string& out_message) {
        u_int32_t length = 0;

        // Receive length
        ssize_t received = recv(socket_fd, &length, sizeof(length), MSG_WAITALL);
        if (received != sizeof(length)) {
            return false;
        }

        // Receive actual message
        out_message.resize(length);

        received = recv(
            socket_fd,
            out_message.data(),
            length,
            MSG_WAITALL
        );

        if (received != static_cast<ssize_t>(length)) {
            return false;
        }

        return true;
    }
} // namespace harmony