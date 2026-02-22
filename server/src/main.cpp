// ─────────────────────────────────────────────────────────────────────
// Harmony Server – main.cpp
//
// Entry point: spins up the io_context and starts the TCP server.
// ─────────────────────────────────────────────────────────────────────

#include "server.hpp"
#include <harmony/message.hpp>
#include <iostream>
#include <csignal>

static boost::asio::io_context* g_io = nullptr;

void signal_handler(int /*sig*/)
{
    std::cout << "\n[server] Shutting down...\n";
    if (g_io) g_io->stop();
}

int main(int argc, char* argv[])
{
    try {
        unsigned short port = harmony::DEFAULT_PORT;
        if (argc >= 2)
            port = static_cast<unsigned short>(std::stoi(argv[1]));

        // TODO 4c: Accept optional DB path argument:
        //   std::string db_path = "harmony.db";
        //   if (argc >= 3) db_path = argv[2];
        //   Then pass it: Server server(io, port, db_path);

        boost::asio::io_context io;
        g_io = &io;

        std::signal(SIGINT,  signal_handler);
        std::signal(SIGTERM, signal_handler);

        harmony::server::Server server(io, port);

        std::cout << "[server] Harmony server v0.1 – press Ctrl+C to stop.\n";
        io.run();
    }
    catch (const std::exception& e) {
        std::cerr << "[server] Fatal: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
