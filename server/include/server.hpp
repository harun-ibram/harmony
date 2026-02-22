#pragma once
// ─────────────────────────────────────────────────────────────────────
// Harmony Server – server.hpp
//
// Accepts TCP connections and manages active sessions.
// Day 2: JSON dispatch, user registry, group membership.
// ─────────────────────────────────────────────────────────────────────

#ifndef HARMONY_SERVER_HPP
#define HARMONY_SERVER_HPP

#include <boost/asio.hpp>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <string>
#include <cstdint>
#include "session.hpp"
#include <harmony/protocol.hpp>
// TODO 4a: #include "harmony_store.hpp" and add std::unique_ptr<SqliteStore> store_ member

namespace harmony::server {

class Server {
public:
    Server(boost::asio::io_context& io, unsigned short port);
    // TODO 4a: Add overload: Server(io_context&, port, db_path) that creates SqliteStore

    /// Broadcast a JSON envelope to every connected client.
    void broadcast(const std::string& msg);

    /// Send a raw string (serialized JSON) to a specific session.
    void send_to(Session::Pointer target, const std::string& msg);

    /// Send a JSON envelope to a specific session.
    void send_json(Session::Pointer target, const proto::json& env);

private:
    void do_accept();

    // ── Dispatch ───────────────────────────────────────────────────
    void on_message(Session::Pointer sender, const std::string& raw);
    void on_disconnect(Session::Pointer session);

    // ── Handlers per message type ──────────────────────────────────
    void handle_ping       (Session::Pointer sender, const proto::json& env);
    void handle_login_req  (Session::Pointer sender, const proto::json& env);
    void handle_dm_send    (Session::Pointer sender, const proto::json& env);
    void handle_group_join (Session::Pointer sender, const proto::json& env);
    void handle_group_leave(Session::Pointer sender, const proto::json& env);
    void handle_group_send (Session::Pointer sender, const proto::json& env);

    // TODO 11a: Declare new handler methods:
    //   void handle_dm_history_req   (Session::Pointer sender, const proto::json& env);
    //   void handle_group_history_req(Session::Pointer sender, const proto::json& env);

    // ── State ──────────────────────────────────────────────────────
    tcp::acceptor                acceptor_;
    std::set<Session::Pointer>   sessions_;          // all connections

    // User registry:  user_id ↔ session
    std::unordered_map<std::string, Session::Pointer> users_;          // user_id → session
    std::uint32_t next_user_id_ = 1;  // TODO 5b: Remove this — DB assigns stable user_ids now

    // TODO 4a: Add std::unique_ptr<SqliteStore> store_ here

    // Group membership: group_id → set of sessions
    std::unordered_map<std::string, std::set<Session::Pointer>> groups_;
};

} // namespace harmony::server

#endif // HARMONY_SERVER_HPP
