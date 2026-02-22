#pragma once
// ─────────────────────────────────────────────────────────────────────
// Harmony Client – client.hpp
//
// Async TCP client.  Connects to the Harmony server, sends and
// receives length-prefixed messages via Boost.Asio.
// Day 2: typed API (login, DM, group) and JSON inbound dispatch.
// ─────────────────────────────────────────────────────────────────────

#ifndef HARMONY_CLIENT_HPP
#define HARMONY_CLIENT_HPP

#include <boost/asio.hpp>
#include <deque>
#include <functional>
#include <string>
#include <harmony/message.hpp>
#include <harmony/protocol.hpp>

namespace harmony::client {

using boost::asio::ip::tcp;

class Client {
public:
    // ── Callback types ─────────────────────────────────────────────
    using RawMessageCallback = std::function<void(const std::string&)>;
    using ConnectCallback    = std::function<void(bool /*success*/)>;
    using JsonCallback       = std::function<void(const proto::json&)>;

    Client(boost::asio::io_context& io,
           const std::string& host,
           unsigned short port);

    // ── Raw send (low-level) ───────────────────────────────────────
    void send(const std::string& msg);

    // ── Typed Day 2 API ────────────────────────────────────────────
    void login(const std::string& username);
    void send_dm(const std::string& to_user_id, const std::string& text);
    void join_group(const std::string& group_id);
    void leave_group(const std::string& group_id);
    void send_group(const std::string& group_id, const std::string& text);
    void send_ping();

    // TODO 12a: Add history request methods:
    //   void request_dm_history(const std::string& peer_user_id, int limit = 50);
    //   void request_group_history(const std::string& group_id, int limit = 50);

    /// Close the connection gracefully.
    void close();

    // ── Callbacks ──────────────────────────────────────────────────
    void set_message_handler(RawMessageCallback cb) { on_message_     = std::move(cb); }
    void set_connect_handler(ConnectCallback cb)    { on_connect_     = std::move(cb); }

    /// Called for every inbound JSON envelope (Day 2).
    void set_json_handler(JsonCallback cb)          { on_json_        = std::move(cb); }

    /// Convenience: specific event callbacks.
    void set_login_handler(JsonCallback cb)         { on_login_res_   = std::move(cb); }
    void set_dm_handler(JsonCallback cb)            { on_dm_msg_      = std::move(cb); }
    void set_group_msg_handler(JsonCallback cb)     { on_group_msg_   = std::move(cb); }
    void set_error_handler(JsonCallback cb)         { on_error_       = std::move(cb); }

    // TODO 12a: Add history callbacks:
    //   void set_dm_history_handler(JsonCallback cb)    { on_dm_history_  = std::move(cb); }
    //   void set_group_history_handler(JsonCallback cb) { on_group_history_ = std::move(cb); }

    bool is_connected() const { return connected_; }

    /// After successful login.res, these are populated.
    const std::string& user_id() const  { return user_id_; }
    const std::string& username() const { return username_; }
    bool is_logged_in() const           { return !user_id_.empty(); }

private:
    void do_connect(const tcp::resolver::results_type& endpoints);
    void do_read_header();
    void do_read_body();
    void do_write();
    void dispatch_json(const proto::json& env);

    boost::asio::io_context& io_;
    tcp::socket              socket_;
    ChatMessage              read_msg_;
    std::deque<ChatMessage>  write_queue_;
    bool                     connected_ = false;

    // Identity (set after login.res)
    std::string user_id_;
    std::string username_;

    // Callbacks
    RawMessageCallback on_message_;
    ConnectCallback    on_connect_;
    JsonCallback       on_json_;
    JsonCallback       on_login_res_;
    JsonCallback       on_dm_msg_;
    JsonCallback       on_group_msg_;
    JsonCallback       on_error_;
    // TODO 12a: Add callback members:
    //   JsonCallback on_dm_history_;
    //   JsonCallback on_group_history_;
};

} // namespace harmony::client

#endif // HARMONY_CLIENT_HPP
