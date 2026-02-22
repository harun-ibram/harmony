// ─────────────────────────────────────────────────────────────────────
// Harmony Server – server.cpp
//
// Day 2: JSON dispatch, login, DM routing, group messaging.
// ─────────────────────────────────────────────────────────────────────

#include "server.hpp"
#include <iostream>

namespace harmony::server {

using json = proto::json;

// TODO 4b: Add a second constructor Server(io, port, db_path) that:
//   - Creates store_ = std::make_unique<SqliteStore>(db_path)
//   - The default constructor below should delegate with db_path="harmony.db"
Server::Server(boost::asio::io_context& io, unsigned short port)
    : acceptor_(io, tcp::endpoint(tcp::v4(), port))
{
    std::cout << "[server] Listening on port " << port << "\n";
    do_accept();
}

// ── Accept loop ───────────────────────────────────────────────────

void Server::do_accept()
{
    acceptor_.async_accept(
        [this](boost::system::error_code ec, tcp::socket socket) {
            if (!ec) {
                auto session = Session::create(std::move(socket));
                std::cout << "[server] New connection from "
                          << session->remote_address() << "\n";

                session->set_message_handler(
                    [this](Session::Pointer s, const std::string& body) {
                        on_message(s, body);
                    });

                session->set_disconnect_handler(
                    [this](Session::Pointer s) {
                        on_disconnect(s);
                    });

                sessions_.insert(session);
                session->start();
            } else {
                std::cerr << "[server] Accept error: " << ec.message() << "\n";
            }

            do_accept();
        });
}

// ── Disconnect cleanup ────────────────────────────────────────────

void Server::on_disconnect(Session::Pointer session)
{
    // Remove from user registry
    if (session->is_logged_in()) {
        users_.erase(session->user_id());
        std::cout << "[server] User '" << session->username()
                  << "' (id=" << session->user_id() << ") disconnected.\n";

        // Remove from all groups
        for (auto& [gid, members] : groups_) {
            members.erase(session);
        }
    }

    sessions_.erase(session);
    std::cout << "[server] Session removed. Active clients: "
              << sessions_.size() << "\n";
}

// ── JSON dispatch ─────────────────────────────────────────────────

void Server::on_message(Session::Pointer sender, const std::string& raw)
{
    // Try to parse as JSON
    auto [env, err] = proto::parse(raw);

    if (!env.is_null()) {
        // Valid JSON envelope — dispatch on type
        std::string t = proto::get_type(env);
        std::cout << "[server] JSON msg type='" << t
                  << "' from " << sender->remote_address() << "\n";

        if      (t == proto::type::PING)        handle_ping(sender, env);
        else if (t == proto::type::LOGIN_REQ)   handle_login_req(sender, env);
        else if (t == proto::type::DM_SEND)     handle_dm_send(sender, env);
        else if (t == proto::type::GROUP_JOIN)   handle_group_join(sender, env);
        else if (t == proto::type::GROUP_LEAVE)  handle_group_leave(sender, env);
        else if (t == proto::type::GROUP_SEND)   handle_group_send(sender, env);
        // TODO 11b: Add dispatch cases for dm.history.req and group.history.req
        //   else if (t == proto::type::DM_HISTORY_REQ)    handle_dm_history_req(sender, env);
        //   else if (t == proto::type::GROUP_HISTORY_REQ) handle_group_history_req(sender, env);
        else {
            // Unknown type
            auto resp = proto::make_error(
                proto::get_id(env), "unknown_type",
                "Unknown message type: " + t);
            send_json(sender, resp);
        }
    } else {
        // Not JSON — Day 1 backward compat: plain-text ping/echo
        if (raw == "ping") {
            send_to(sender, "pong");
        } else {
            send_to(sender, raw);
        }
    }
}

// ── Handler: ping ─────────────────────────────────────────────────

void Server::handle_ping(Session::Pointer sender, const json& env)
{
    auto resp = proto::make_response(proto::type::PONG, proto::get_id(env));
    resp["ts"] = proto::now_ms();
    send_json(sender, resp);
}

// ── Handler: login.req ────────────────────────────────────────────

void Server::handle_login_req(Session::Pointer sender, const json& env)
{
    std::string req_id = proto::get_id(env);

    // Already logged in?
    if (sender->is_logged_in()) {
        send_json(sender, proto::make_error(req_id, "already_logged_in",
                  "You are already logged in as " + sender->username()));
        return;
    }

    // Extract username from body
    std::string username;
    if (env.contains("body") && env["body"].contains("username")) {
        username = env["body"]["username"].get<std::string>();
    }

    if (username.empty()) {
        send_json(sender, proto::make_error(req_id, "bad_request",
                  "Missing 'body.username'"));
        return;
    }

    // TODO 5a: Replace this block with DB-backed login:
    //   - Call store_->get_or_create_user(username) to get a stable user_id
    //   - Check if that user_id is already in users_ (prevents double-login)
    //   - Remove the next_user_id_++ counter entirely

    // Check if username is already taken (by scanning active users)
    for (auto& [uid, sess] : users_) {
        if (sess->username() == username) {
            send_json(sender, proto::make_error(req_id, "username_taken",
                      "Username '" + username + "' is already in use"));
            return;
        }
    }

    // Assign a user_id
    std::string user_id = "u" + std::to_string(next_user_id_++);
    sender->set_identity(user_id, username);
    users_[user_id] = sender;

    std::cout << "[server] Login: '" << username
              << "' assigned id=" << user_id << "\n";

    // Send login.res
    auto resp = proto::make_response(proto::type::LOGIN_RES, req_id);
    resp["body"] = {{"user_id", user_id}, {"username", username}};
    resp["ts"]   = proto::now_ms();
    send_json(sender, resp);

    // TODO 6c: After sending login.res, auto-rejoin persisted groups:
    //   - Call store_->list_groups_for_user(user_id)
    //   - For each group, insert session into groups_[group_id]
    //   - Optionally include group list in login.res body

    // TODO 8a: After login + auto-rejoin, deliver undelivered DMs:
    //   - Call store_->fetch_undelivered_dms(user_id, 100)
    //   - For each MessageRow, build a dm.msg envelope and send_json(sender, ...)
    //   - Collect row IDs and call store_->mark_dms_delivered(ids)
}

// ── Handler: dm.send ──────────────────────────────────────────────

void Server::handle_dm_send(Session::Pointer sender, const json& env)
{
    std::string req_id = proto::get_id(env);

    // Must be logged in
    if (!sender->is_logged_in()) {
        send_json(sender, proto::make_error(req_id, "not_logged_in",
                  "You must log in before sending messages"));
        return;
    }

    // Extract target user_id
    std::string to_user_id;
    if (env.contains("to") && env["to"].contains("user_id")) {
        to_user_id = env["to"]["user_id"].get<std::string>();
    }
    if (to_user_id.empty()) {
        send_json(sender, proto::make_error(req_id, "bad_request",
                  "Missing 'to.user_id'"));
        return;
    }

    // Extract text
    std::string text;
    if (env.contains("body") && env["body"].contains("text")) {
        text = env["body"]["text"].get<std::string>();
    }
    if (text.empty()) {
        send_json(sender, proto::make_error(req_id, "bad_request",
                  "Missing 'body.text'"));
        return;
    }

    // TODO 7a: Replace this block with DB-backed DM handling:
    //   - Check store_->user_exists(to_user_id) instead of requiring online
    //   - Always call store_->store_dm(...) to persist the message
    //   - If recipient is in users_ (online): send dm.msg AND mark_dms_delivered
    //   - If offline: return success to sender (DM is queued for later delivery)

    // Find recipient
    auto it = users_.find(to_user_id);
    if (it == users_.end()) {
        send_json(sender, proto::make_error(req_id, "user_not_found",
                  "User '" + to_user_id + "' is not online"));
        return;
    }

    // Build dm.msg for recipient
    auto msg = proto::make_envelope(proto::type::DM_MSG);
    msg["from"] = {{"user_id", sender->user_id()}, {"username", sender->username()}};
    msg["to"]   = {{"user_id", to_user_id}};
    msg["body"] = {{"text", text}};
    msg["ts"]   = proto::now_ms();
    send_json(it->second, msg);

    std::cout << "[server] DM: " << sender->username()
              << " -> " << it->second->username() << "\n";
}

// ── Handler: group.join ───────────────────────────────────────────

void Server::handle_group_join(Session::Pointer sender, const json& env)
{
    std::string req_id = proto::get_id(env);

    if (!sender->is_logged_in()) {
        send_json(sender, proto::make_error(req_id, "not_logged_in",
                  "You must log in first"));
        return;
    }

    std::string group_id;
    if (env.contains("to") && env["to"].contains("group_id")) {
        group_id = env["to"]["group_id"].get<std::string>();
    }
    if (group_id.empty()) {
        send_json(sender, proto::make_error(req_id, "bad_request",
                  "Missing 'to.group_id'"));
        return;
    }

    groups_[group_id].insert(sender);
    // TODO 6a: Persist membership: store_->add_group_member(sender->user_id(), group_id)
    std::cout << "[server] " << sender->username()
              << " joined group '" << group_id << "' (now "
              << groups_[group_id].size() << " members)\n";

    // Acknowledge
    auto resp = proto::make_response(proto::type::GROUP_JOIN, req_id);
    resp["to"]   = {{"group_id", group_id}};
    resp["ts"]   = proto::now_ms();
    send_json(sender, resp);
}

// ── Handler: group.leave ──────────────────────────────────────────

void Server::handle_group_leave(Session::Pointer sender, const json& env)
{
    std::string req_id = proto::get_id(env);

    if (!sender->is_logged_in()) {
        send_json(sender, proto::make_error(req_id, "not_logged_in",
                  "You must log in first"));
        return;
    }

    std::string group_id;
    if (env.contains("to") && env["to"].contains("group_id")) {
        group_id = env["to"]["group_id"].get<std::string>();
    }
    if (group_id.empty()) {
        send_json(sender, proto::make_error(req_id, "bad_request",
                  "Missing 'to.group_id'"));
        return;
    }

    auto git = groups_.find(group_id);
    if (git != groups_.end()) {
        git->second.erase(sender);
        if (git->second.empty()) groups_.erase(git);
    }
    // TODO 6b: Persist removal: store_->remove_group_member(sender->user_id(), group_id)

    std::cout << "[server] " << sender->username()
              << " left group '" << group_id << "'\n";

    auto resp = proto::make_response(proto::type::GROUP_LEAVE, req_id);
    resp["to"]   = {{"group_id", group_id}};
    resp["ts"]   = proto::now_ms();
    send_json(sender, resp);
}

// ── Handler: group.send ───────────────────────────────────────────

void Server::handle_group_send(Session::Pointer sender, const json& env)
{
    std::string req_id = proto::get_id(env);

    if (!sender->is_logged_in()) {
        send_json(sender, proto::make_error(req_id, "not_logged_in",
                  "You must log in first"));
        return;
    }

    std::string group_id;
    if (env.contains("to") && env["to"].contains("group_id")) {
        group_id = env["to"]["group_id"].get<std::string>();
    }
    if (group_id.empty()) {
        send_json(sender, proto::make_error(req_id, "bad_request",
                  "Missing 'to.group_id'"));
        return;
    }

    std::string text;
    if (env.contains("body") && env["body"].contains("text")) {
        text = env["body"]["text"].get<std::string>();
    }
    if (text.empty()) {
        send_json(sender, proto::make_error(req_id, "bad_request",
                  "Missing 'body.text'"));
        return;
    }

    auto git = groups_.find(group_id);
    if (git == groups_.end() || git->second.find(sender) == git->second.end()) {
        send_json(sender, proto::make_error(req_id, "not_in_group",
                  "You are not a member of group '" + group_id + "'"));
        return;
    }

    // Build group.msg — deliver to all members except sender
    auto msg = proto::make_envelope(proto::type::GROUP_MSG);
    msg["from"] = {{"user_id", sender->user_id()}, {"username", sender->username()}};
    msg["to"]   = {{"group_id", group_id}};
    msg["body"] = {{"text", text}};
    msg["ts"]   = proto::now_ms();

    std::string serialized = proto::serialize(msg);
    for (auto& member : git->second) {
        if (member != sender) {
            send_to(member, serialized);
        }
    }

    // TODO 9a: Persist group message: store_->store_group_msg(sender->user_id(), group_id, text, proto::get_id(env), proto::now_ms())

    std::cout << "[server] Group msg in '" << group_id
              << "' from " << sender->username() << "\n";
}

// TODO 11c: Implement handle_dm_history_req:
//   - Validate sender is logged in
//   - Extract peer_user_id and limit from body
//   - Call store_->fetch_dm_history(sender->user_id(), peer_user_id, limit)
//   - Build dm.history.res with body.messages = [...]
//   - send_json(sender, resp)

// TODO 11d: Implement handle_group_history_req:
//   - Validate logged in + group membership
//   - Extract group_id and limit from body
//   - Call store_->fetch_group_history(group_id, limit)
//   - Build group.history.res with body.messages = [...]
//   - send_json(sender, resp)

// ── Delivery helpers ──────────────────────────────────────────────

void Server::broadcast(const std::string& msg)
{
    for (auto& s : sessions_)
        s->deliver(msg);
}

void Server::send_to(Session::Pointer target, const std::string& msg)
{
    target->deliver(msg);
}

void Server::send_json(Session::Pointer target, const json& env)
{
    target->deliver(proto::serialize(env));
}

} // namespace harmony::server
