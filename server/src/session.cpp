// ─────────────────────────────────────────────────────────────────────
// Harmony Server – session.cpp
// ─────────────────────────────────────────────────────────────────────

#include "session.hpp"
#include <iostream>

namespace harmony::server {

Session::Session(tcp::socket socket)
    : socket_(std::move(socket))
{}

std::string Session::remote_address() const
{
    try {
        auto ep = socket_.remote_endpoint();
        return ep.address().to_string() + ":" + std::to_string(ep.port());
    } catch (...) {
        return "(unknown)";
    }
}

// ── Start the read chain ──────────────────────────────────────────

void Session::start()
{
    do_read_header();
}

// ── Read the 4-byte length header ─────────────────────────────────

void Session::do_read_header()
{
    auto self = shared_from_this();
    boost::asio::async_read(
        socket_,
        boost::asio::buffer(read_msg_.data(), harmony::HEADER_LENGTH),
        [this, self](boost::system::error_code ec, std::size_t /*length*/) {
            if (!ec && read_msg_.decode_header()) {
                do_read_body();
            } else {
                std::cout << "[server] Client disconnected: "
                          << remote_address() << "\n";
                if (on_disconnect_) on_disconnect_(self);
            }
        });
}

// ── Read the body ─────────────────────────────────────────────────

void Session::do_read_body()
{
    auto self = shared_from_this();
    boost::asio::async_read(
        socket_,
        boost::asio::buffer(read_msg_.body(), read_msg_.body_length()),
        [this, self](boost::system::error_code ec, std::size_t /*length*/) {
            if (!ec) {
                std::string body = read_msg_.body_as_string();
                std::cout << "[server] Received from "
                          << remote_address() << ": " << body << "\n";

                if (on_message_) on_message_(self, body);

                do_read_header();   // keep reading
            } else {
                std::cout << "[server] Read error: " << ec.message() << "\n";
                if (on_disconnect_) on_disconnect_(self);
            }
        });
}

// ── Write queue ───────────────────────────────────────────────────

void Session::deliver(const std::string& msg)
{
    ChatMessage out;
    out.set_body(msg);

    bool write_in_progress = !write_queue_.empty();
    write_queue_.push_back(std::move(out));

    if (!write_in_progress)
        do_write();
}

void Session::do_write()
{
    auto self = shared_from_this();
    boost::asio::async_write(
        socket_,
        boost::asio::buffer(write_queue_.front().data(),
                            write_queue_.front().length()),
        [this, self](boost::system::error_code ec, std::size_t /*length*/) {
            if (!ec) {
                write_queue_.pop_front();
                if (!write_queue_.empty())
                    do_write();
            } else {
                std::cerr << "[server] Write error: " << ec.message() << "\n";
                if (on_disconnect_) on_disconnect_(self);
            }
        });
}

} // namespace harmony::server
