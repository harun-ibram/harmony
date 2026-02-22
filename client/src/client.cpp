// ─────────────────────────────────────────────────────────────────────
// Harmony Client – client.cpp
//
// Day 2: typed API, JSON inbound dispatch.
// ─────────────────────────────────────────────────────────────────────

#include "client.hpp"
#include <iostream>

namespace harmony::client {

Client::Client(boost::asio::io_context& io,
               const std::string& host,
               unsigned short port)
    : io_(io), socket_(io)
{
    tcp::resolver resolver(io);
    auto endpoints = resolver.resolve(host, std::to_string(port));
    do_connect(endpoints);
}

// ── Connect ───────────────────────────────────────────────────────

void Client::do_connect(const tcp::resolver::results_type& endpoints)
{
    boost::asio::async_connect(
        socket_, endpoints,
        [this](boost::system::error_code ec, const tcp::endpoint& ep) {
            if (!ec) {
                connected_ = true;
                std::cout << "[client] Connected to "
                          << ep.address().to_string() << ":"
                          << ep.port() << "\n";
                if (on_connect_) on_connect_(true);
                do_read_header();
            } else {
                std::cerr << "[client] Connection failed: "
                          << ec.message() << "\n";
                if (on_connect_) on_connect_(false);
            }
        });
}

// ── Read ──────────────────────────────────────────────────────────

void Client::do_read_header()
{
    boost::asio::async_read(
        socket_,
        boost::asio::buffer(read_msg_.data(), harmony::HEADER_LENGTH),
        [this](boost::system::error_code ec, std::size_t /*length*/) {
            if (!ec && read_msg_.decode_header()) {
                do_read_body();
            } else {
                std::cout << "[client] Server disconnected.\n";
                connected_ = false;
                socket_.close();
            }
        });
}

void Client::do_read_body()
{
    boost::asio::async_read(
        socket_,
        boost::asio::buffer(read_msg_.body(), read_msg_.body_length()),
        [this](boost::system::error_code ec, std::size_t /*length*/) {
            if (!ec) {
                std::string body = read_msg_.body_as_string();
                std::cout << "[client] Received: " << body << "\n";

                // Fire raw callback
                if (on_message_) on_message_(body);

                // Try JSON dispatch
                auto [env, err] = proto::parse(body);
                if (!env.is_null()) {
                    dispatch_json(env);
                }

                do_read_header();
            } else {
                std::cerr << "[client] Read error: " << ec.message() << "\n";
                connected_ = false;
                socket_.close();
            }
        });
}

// ── JSON inbound dispatch ─────────────────────────────────────────

void Client::dispatch_json(const proto::json& env)
{
    // Fire the generic JSON handler
    if (on_json_) on_json_(env);

    std::string t = proto::get_type(env);

    if (t == proto::type::LOGIN_RES) {
        // Store identity
        if (env.contains("body")) {
            if (env["body"].contains("user_id"))
                user_id_ = env["body"]["user_id"].get<std::string>();
            if (env["body"].contains("username"))
                username_ = env["body"]["username"].get<std::string>();
        }
        if (on_login_res_) on_login_res_(env);
    }
    else if (t == proto::type::DM_MSG) {
        if (on_dm_msg_) on_dm_msg_(env);
    }
    else if (t == proto::type::GROUP_MSG) {
        if (on_group_msg_) on_group_msg_(env);
    }
    else if (t == proto::type::ERROR_MSG) {
        if (on_error_) on_error_(env);
    }
    // TODO 12b: Add dispatch cases for history responses:
    //   else if (t == proto::type::DM_HISTORY_RES) {
    //       if (on_dm_history_) on_dm_history_(env);
    //   }
    //   else if (t == proto::type::GROUP_HISTORY_RES) {
    //       if (on_group_history_) on_group_history_(env);
    //   }
    // pong, group.join ack, group.leave ack — handled by generic on_json_
}

// ── Typed send API ────────────────────────────────────────────────

void Client::login(const std::string& username)
{
    send(proto::serialize(proto::login_req(username)));
}

void Client::send_dm(const std::string& to_user_id, const std::string& text)
{
    send(proto::serialize(proto::dm_send(to_user_id, text)));
}

void Client::join_group(const std::string& group_id)
{
    send(proto::serialize(proto::group_join(group_id)));
}

void Client::leave_group(const std::string& group_id)
{
    send(proto::serialize(proto::group_leave(group_id)));
}

void Client::send_group(const std::string& group_id, const std::string& text)
{
    send(proto::serialize(proto::group_send(group_id, text)));
}

void Client::send_ping()
{
    send(proto::serialize(proto::make_envelope(proto::type::PING)));
}

// TODO 12b: Implement history request methods:
//   void Client::request_dm_history(const std::string& peer_user_id, int limit) {
//       send(proto::serialize(proto::dm_history_req(peer_user_id, limit)));
//   }
//   void Client::request_group_history(const std::string& group_id, int limit) {
//       send(proto::serialize(proto::group_history_req(group_id, limit)));
//   }

// ── Write ─────────────────────────────────────────────────────────

void Client::send(const std::string& msg)
{
    boost::asio::post(io_, [this, msg]() {
        ChatMessage out;
        out.set_body(msg);

        bool write_in_progress = !write_queue_.empty();
        write_queue_.push_back(std::move(out));

        if (!write_in_progress)
            do_write();
    });
}

void Client::do_write()
{
    boost::asio::async_write(
        socket_,
        boost::asio::buffer(write_queue_.front().data(),
                            write_queue_.front().length()),
        [this](boost::system::error_code ec, std::size_t /*length*/) {
            if (!ec) {
                write_queue_.pop_front();
                if (!write_queue_.empty())
                    do_write();
            } else {
                std::cerr << "[client] Write error: " << ec.message() << "\n";
                connected_ = false;
                socket_.close();
            }
        });
}

// ── Close ─────────────────────────────────────────────────────────

void Client::close()
{
    boost::asio::post(io_, [this]() {
        connected_ = false;
        socket_.close();
    });
}

} // namespace harmony::client
