// ─────────────────────────────────────────────────────────────────────
// Harmony Client – main.cpp
//
// Interactive console client.  Day 2: typed commands.
//   /login <username>
//   /dm <user_id> <message>
//   /join <group_id>
//   /leave <group_id>
//   /group <group_id> <message>
//   /ping
//   /quit
// ─────────────────────────────────────────────────────────────────────

#include "client.hpp"
#include <harmony/message.hpp>
#include <harmony/protocol.hpp>
#include <iostream>
#include <thread>
#include <string>
#include <sstream>

int main(int argc, char* argv[])
{
    try {
        std::string host = "127.0.0.1";
        unsigned short port = harmony::DEFAULT_PORT;

        if (argc >= 2) host = argv[1];
        if (argc >= 3) port = static_cast<unsigned short>(std::stoi(argv[2]));

        boost::asio::io_context io;

        harmony::client::Client client(io, host, port);

        client.set_login_handler([](const harmony::proto::json& env) {
            std::cout << "\n[login] Success! user_id="
                      << env["body"]["user_id"].get<std::string>()
                      << "  username="
                      << env["body"]["username"].get<std::string>()
                      << "\n> " << std::flush;
        });

        client.set_dm_handler([](const harmony::proto::json& env) {
            std::cout << "\n[DM from "
                      << env["from"]["username"].get<std::string>() << "] "
                      << env["body"]["text"].get<std::string>()
                      << "\n> " << std::flush;
        });

        client.set_group_msg_handler([](const harmony::proto::json& env) {
            std::cout << "\n[Group:"
                      << env["to"]["group_id"].get<std::string>()
                      << " from " << env["from"]["username"].get<std::string>()
                      << "] " << env["body"]["text"].get<std::string>()
                      << "\n> " << std::flush;
        });

        client.set_error_handler([](const harmony::proto::json& env) {
            std::cout << "\n[ERROR] "
                      << env["error"]["code"].get<std::string>() << ": "
                      << env["error"]["message"].get<std::string>()
                      << "\n> " << std::flush;
        });

        // Run io_context on a background thread
        std::thread io_thread([&io]() { io.run(); });

        std::cout << "Harmony Client v2 – Commands:\n"
                  << "  /login <username>\n"
                  << "  /dm <user_id> <message>\n"
                  << "  /join <group_id>\n"
                  << "  /leave <group_id>\n"
                  << "  /group <group_id> <message>\n"
                  << "  /ping\n"
                  << "  /quit\n\n> " << std::flush;

        std::string line;
        while (std::getline(std::cin, line)) {
            if (line.empty()) { std::cout << "> " << std::flush; continue; }

            std::istringstream iss(line);
            std::string cmd;
            iss >> cmd;

            if (cmd == "/quit" || cmd == "/exit") {
                break;
            } else if (cmd == "/login") {
                std::string username;
                iss >> username;
                if (username.empty()) {
                    std::cout << "Usage: /login <username>\n> " << std::flush;
                } else {
                    client.login(username);
                }
            } else if (cmd == "/dm") {
                std::string uid, text;
                iss >> uid;
                std::getline(iss, text);
                // trim leading space
                if (!text.empty() && text[0] == ' ') text = text.substr(1);
                if (uid.empty() || text.empty()) {
                    std::cout << "Usage: /dm <user_id> <message>\n> " << std::flush;
                } else {
                    client.send_dm(uid, text);
                }
            } else if (cmd == "/join") {
                std::string gid;
                iss >> gid;
                if (gid.empty()) {
                    std::cout << "Usage: /join <group_id>\n> " << std::flush;
                } else {
                    client.join_group(gid);
                }
            } else if (cmd == "/leave") {
                std::string gid;
                iss >> gid;
                if (gid.empty()) {
                    std::cout << "Usage: /leave <group_id>\n> " << std::flush;
                } else {
                    client.leave_group(gid);
                }
            } else if (cmd == "/group") {
                std::string gid, text;
                iss >> gid;
                std::getline(iss, text);
                if (!text.empty() && text[0] == ' ') text = text.substr(1);
                if (gid.empty() || text.empty()) {
                    std::cout << "Usage: /group <group_id> <message>\n> " << std::flush;
                } else {
                    client.send_group(gid, text);
                }
            } else if (cmd == "/ping") {
                client.send_ping();
            }
            // TODO 12c: Add history console commands:
            //   else if (cmd == "/history") {
            //       std::string subcmd, id;
            //       int limit = 50;
            //       iss >> subcmd >> id;
            //       if (iss >> limit) {} // optional limit
            //       if (subcmd == "dm")    client.request_dm_history(id, limit);
            //       if (subcmd == "group") client.request_group_history(id, limit);
            //   }
            else {
                std::cout << "Unknown command. Type /quit to exit.\n> " << std::flush;
            }

            std::cout << "> " << std::flush;
        }

        client.close();
        io_thread.join();
    }
    catch (const std::exception& e) {
        std::cerr << "[client] Fatal: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
