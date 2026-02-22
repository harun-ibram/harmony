#pragma once
// ─────────────────────────────────────────────────────────────────────
// Harmony Server – session.hpp
//
// One Session object per connected client.  Reads length-prefixed
// messages, processes them, and writes responses asynchronously.
// Day 2: carries per-connection identity (user_id, username).
// ─────────────────────────────────────────────────────────────────────

#ifndef HARMONY_SERVER_SESSION_HPP
#define HARMONY_SERVER_SESSION_HPP

#include <boost/asio.hpp>
#include <deque>
#include <memory>
#include <string>
#include <functional>
#include <harmony/message.hpp>

namespace harmony::server {

using boost::asio::ip::tcp;

class Session : public std::enable_shared_from_this<Session> {
public:
    using Pointer = std::shared_ptr<Session>;

    /// Callback fired when a complete message arrives.
    using MessageHandler = std::function<void(Pointer, const std::string&)>;

    /// Callback fired when the session disconnects.
    using DisconnectHandler = std::function<void(Pointer)>;

    static Pointer create(tcp::socket socket)
    {
        return Pointer(new Session(std::move(socket)));
    }

    /// Start reading from the socket.
    void start();

    /// Queue a message for asynchronous delivery.
    void deliver(const std::string& msg);

    /// Set handlers (call before start()).
    void set_message_handler(MessageHandler handler)       { on_message_    = std::move(handler); }
    void set_disconnect_handler(DisconnectHandler handler)  { on_disconnect_ = std::move(handler); }

    tcp::socket& socket() { return socket_; }

    std::string remote_address() const;

    // Identity

    bool is_logged_in() const           { return logged_in_; }

    const std::string& user_id() const  { return user_id_; }
    const std::string& username() const { return username_; }

    void set_identity(const std::string& user_id, const std::string& username)
    {
        user_id_   = user_id;
        username_  = username;
        logged_in_ = true;
    }

private:
    explicit Session(tcp::socket socket);

    void do_read_header();
    void do_read_body();
    void do_write();

    tcp::socket         socket_;
    ChatMessage         read_msg_;
    std::deque<ChatMessage> write_queue_;

    MessageHandler      on_message_;
    DisconnectHandler   on_disconnect_;

    // Identity state
    bool        logged_in_ = false;
    std::string user_id_;
    std::string username_;
};

} // namespace harmony::server

#endif // HARMONY_SERVER_SESSION_HPP
