// ─────────────────────────────────────────────────────────────────────
// Harmony – tests/ping_test.cpp
//
// Day 2 integration tests using the REAL Server + Client classes.
//
// Tests:
//   1) JSON ping → pong (with reply_to correlation)
//   2) Login flow (username → user_id assignment)
//   3) DM routing (Alice → Bob by user_id)
//   4) Group messaging (Alice + Bob in group; Alice sends, Bob receives)
//
// Usage:  ./ping_test          (exits 0 on success, 1 on failure)
// ─────────────────────────────────────────────────────────────────────

#include <server.hpp>
#include <client.hpp>
#include <harmony/protocol.hpp>

#include <iostream>
#include <thread>
#include <chrono>
#include <atomic>
#include <string>
#include <functional>
#include <mutex>
#include <condition_variable>

// ════════════════════════════════════════════════════════════════════
// Test helpers
// ════════════════════════════════════════════════════════════════════

static constexpr unsigned short TEST_PORT = 19876;
static constexpr auto TIMEOUT = std::chrono::seconds(10);

struct TestResult {
    std::mutex              mtx;
    std::condition_variable cv;
    bool                    done  = false;
    bool                    pass  = false;
    std::string             info;

    void succeed(const std::string& msg = "") {
        std::lock_guard lk(mtx);
        pass = true; done = true; info = msg;
        cv.notify_all();
    }
    void fail(const std::string& msg) {
        std::lock_guard lk(mtx);
        pass = false; done = true; info = msg;
        cv.notify_all();
    }
    bool wait() {
        std::unique_lock lk(mtx);
        cv.wait_for(lk, TIMEOUT, [&]{ return done; });
        if (!done) { info = "TIMEOUT"; pass = false; }
        return pass;
    }
};

static void report(const std::string& name, bool passed, const std::string& info)
{
    std::cout << (passed ? "  [PASS] " : "  [FAIL] ") << name;
    if (!info.empty()) std::cout << "  (" << info << ")";
    std::cout << "\n";
}

// ════════════════════════════════════════════════════════════════════
// Test 1: JSON ping → pong
// ════════════════════════════════════════════════════════════════════

static bool test_json_ping(boost::asio::io_context& io)
{
    TestResult r;
    std::string sent_id;

    harmony::client::Client client(io, "127.0.0.1", TEST_PORT);

    client.set_connect_handler([&](bool ok) {
        if (!ok) { r.fail("connect failed"); return; }
        // Send a JSON ping
        auto env = harmony::proto::make_envelope(harmony::proto::type::PING);
        sent_id = harmony::proto::get_id(env);
        client.send(harmony::proto::serialize(env));
    });

    client.set_json_handler([&](const harmony::proto::json& env) {
        std::string t = harmony::proto::get_type(env);
        if (t == harmony::proto::type::PONG) {
            std::string reply = harmony::proto::get_reply_to(env);
            if (reply == sent_id) {
                r.succeed("reply_to matched");
            } else {
                r.fail("reply_to mismatch: expected=" + sent_id + " got=" + reply);
            }
        }
    });

    bool ok = r.wait();
    client.close();
    // Give close a moment to process
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    return ok;
}

// ════════════════════════════════════════════════════════════════════
// Test 2: Login flow
// ════════════════════════════════════════════════════════════════════

static bool test_login(boost::asio::io_context& io, std::string& out_user_id)
{
    TestResult r;

    harmony::client::Client client(io, "127.0.0.1", TEST_PORT);

    client.set_connect_handler([&](bool ok) {
        if (!ok) { r.fail("connect failed"); return; }
        client.login("TestAlice");
    });

    client.set_login_handler([&](const harmony::proto::json& env) {
        if (env.value("ok", false) &&
            env.contains("body") &&
            env["body"].contains("user_id"))
        {
            out_user_id = env["body"]["user_id"].get<std::string>();
            r.succeed("user_id=" + out_user_id);
        } else {
            r.fail("unexpected login.res: " + env.dump());
        }
    });

    client.set_error_handler([&](const harmony::proto::json& env) {
        r.fail("error: " + env.dump());
    });

    bool ok = r.wait();
    client.close();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    return ok;
}

// ════════════════════════════════════════════════════════════════════
// Test 3: DM routing (Alice → Bob)
// ════════════════════════════════════════════════════════════════════

static bool test_dm(boost::asio::io_context& io)
{
    TestResult r;
    std::string alice_id, bob_id;

    // Alice client
    harmony::client::Client alice(io, "127.0.0.1", TEST_PORT);
    // Bob client
    harmony::client::Client bob(io, "127.0.0.1", TEST_PORT);

    // Step sequence: Alice connects → logs in → Bob connects → logs in
    //                → Alice sends DM to Bob → Bob receives it

    std::atomic<int> logins_done{0};

    alice.set_connect_handler([&](bool ok) {
        if (!ok) { r.fail("alice connect failed"); return; }
        alice.login("Alice");
    });

    alice.set_login_handler([&](const harmony::proto::json& env) {
        alice_id = env["body"]["user_id"].get<std::string>();
        if (++logins_done == 2) {
            // Both logged in → Alice sends DM to Bob
            alice.send_dm(bob_id, "Hello Bob!");
        }
    });

    bob.set_connect_handler([&](bool ok) {
        if (!ok) { r.fail("bob connect failed"); return; }
        bob.login("Bob");
    });

    bob.set_login_handler([&](const harmony::proto::json& env) {
        bob_id = env["body"]["user_id"].get<std::string>();
        if (++logins_done == 2) {
            // Both logged in → Alice sends DM to Bob
            alice.send_dm(bob_id, "Hello Bob!");
        }
    });

    bob.set_dm_handler([&](const harmony::proto::json& env) {
        std::string text = env["body"]["text"].get<std::string>();
        std::string from = env["from"]["username"].get<std::string>();
        if (text == "Hello Bob!" && from == "Alice") {
            r.succeed("DM delivered");
        } else {
            r.fail("unexpected DM: " + env.dump());
        }
    });

    alice.set_error_handler([&](const harmony::proto::json& env) {
        r.fail("alice error: " + env.dump());
    });
    bob.set_error_handler([&](const harmony::proto::json& env) {
        r.fail("bob error: " + env.dump());
    });

    bool ok = r.wait();
    alice.close();
    bob.close();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    return ok;
}

// ════════════════════════════════════════════════════════════════════
// Test 4: Group messaging
// ════════════════════════════════════════════════════════════════════

static bool test_group(boost::asio::io_context& io)
{
    TestResult r;
    const std::string GROUP = "testgroup";

    harmony::client::Client alice(io, "127.0.0.1", TEST_PORT);
    harmony::client::Client bob(io, "127.0.0.1", TEST_PORT);

    std::atomic<int> joins_done{0};

    alice.set_connect_handler([&](bool ok) {
        if (!ok) { r.fail("alice connect"); return; }
        alice.login("GAlice");
    });

    alice.set_login_handler([&](const harmony::proto::json&) {
        alice.join_group(GROUP);
    });

    bob.set_connect_handler([&](bool ok) {
        if (!ok) { r.fail("bob connect"); return; }
        bob.login("GBob");
    });

    bob.set_login_handler([&](const harmony::proto::json&) {
        bob.join_group(GROUP);
    });

    // Track group.join acks
    alice.set_json_handler([&](const harmony::proto::json& env) {
        std::string t = harmony::proto::get_type(env);
        if (t == harmony::proto::type::GROUP_JOIN && env.value("ok", false)) {
            if (++joins_done == 2) {
                // Both joined → Alice sends to group
                alice.send_group(GROUP, "Hello Group!");
            }
        }
    });

    bob.set_json_handler([&](const harmony::proto::json& env) {
        std::string t = harmony::proto::get_type(env);
        if (t == harmony::proto::type::GROUP_JOIN && env.value("ok", false)) {
            if (++joins_done == 2) {
                alice.send_group(GROUP, "Hello Group!");
            }
        }
        if (t == harmony::proto::type::GROUP_MSG) {
            std::string text = env["body"]["text"].get<std::string>();
            std::string from = env["from"]["username"].get<std::string>();
            if (text == "Hello Group!" && from == "GAlice") {
                r.succeed("group msg delivered");
            } else {
                r.fail("unexpected group msg: " + env.dump());
            }
        }
    });

    alice.set_error_handler([&](const harmony::proto::json& env) {
        r.fail("alice error: " + env.dump());
    });
    bob.set_error_handler([&](const harmony::proto::json& env) {
        r.fail("bob error: " + env.dump());
    });

    bool ok = r.wait();
    alice.close();
    bob.close();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    return ok;
}

// TODO 13a: Add Test 5 — Stable user_id across restart:
//   - Create a temp DB file path (e.g. "test_day3.db")
//   - Start server with that path. Client "Alice" logs in → capture user_id. Close client. Stop server.
//   - Start NEW server with same DB path. Client "Alice" logs in → capture user_id.
//   - Assert both user_ids are the same.

// TODO 13b: Add Test 6 — Offline DM delivery:
//   - Start server (temp DB). "Alice" logs in, "Bob" logs in → capture bob_id. Bob disconnects.
//   - Alice sends DM to bob_id. Should get success (not error).
//   - Stop server. Start new server (same DB). Bob logs in.
//   - Bob should automatically receive the stored dm.msg after login.

// TODO 13c: Add Test 7 — Group membership persistence:
//   - Start server (temp DB). Alice logs in, joins "devteam". Stop server.
//   - Start new server (same DB). Alice logs in (should auto-rejoin "devteam").
//   - Bob logs in, joins "devteam". Alice sends group.send to "devteam".
//   - Bob should receive the group.msg — proving Alice is still a member.

// TODO 13d: (Optional) Add Test 8 — DM history retrieval:
//   - Alice sends 3 DMs to Bob (both online). Bob requests dm.history.req.
//   - Assert the dm.history.res contains all 3 messages in order.

// ════════════════════════════════════════════════════════════════════
// Main test runner
// ════════════════════════════════════════════════════════════════════

int main()
{
    std::cout << "\n========================================\n"
              << "  Harmony Day 2 Integration Tests\n"
              << "========================================\n\n";

    boost::asio::io_context io;

    // Start the real server
    harmony::server::Server server(io, TEST_PORT);

    // Run io_context on a background thread
    std::thread io_thread([&io]() { io.run(); });

    // Small delay so the server is ready to accept
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    int passed = 0, failed = 0;
    std::string dummy_uid;

    // Test 1: JSON ping
    {
        bool ok = test_json_ping(io);
        report("JSON ping -> pong", ok, "");
        ok ? ++passed : ++failed;
    }

    // Test 2: Login
    {
        bool ok = test_login(io, dummy_uid);
        report("Login flow", ok, dummy_uid);
        ok ? ++passed : ++failed;
    }

    // Test 3: DM routing
    {
        bool ok = test_dm(io);
        report("DM routing (Alice -> Bob)", ok, "");
        ok ? ++passed : ++failed;
    }

    // Test 4: Group messaging
    {
        bool ok = test_group(io);
        report("Group messaging", ok, "");
        ok ? ++passed : ++failed;
    }

    // Summary
    std::cout << "\n========================================\n"
              << "  Results: " << passed << " passed, " << failed << " failed\n"
              << "========================================\n\n";

    io.stop();
    io_thread.join();

    return (failed == 0) ? 0 : 1;
}
