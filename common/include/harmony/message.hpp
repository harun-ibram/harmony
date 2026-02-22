#pragma once
// ─────────────────────────────────────────────────────────────────────
// Harmony – common/include/harmony/message.hpp
//
// Thin, shared wire-protocol helpers.
// Day 1: length-prefixed plain-text frames.
// Day 2: evolves into JSON payloads.
// ─────────────────────────────────────────────────────────────────────

#ifndef HARMONY_MESSAGE_HPP
#define HARMONY_MESSAGE_HPP

#include <cstdint>
#include <cstring>
#include <string>
#include <array>
#include <stdexcept>

namespace harmony {

// ── Wire format ────────────────────────────────────────────────────
//  [4 bytes – uint32 body length, network byte order] [body bytes]
// ───────────────────────────────────────────────────────────────────

constexpr std::size_t HEADER_LENGTH = 4;          // bytes
constexpr std::size_t MAX_BODY_LENGTH = 65536;     // 64 KiB

// ── Encode / Decode helpers ────────────────────────────────────────

/// Encode a body length into a 4-byte header (big-endian).
inline void encode_header(char* buf, std::uint32_t body_len)
{
    buf[0] = static_cast<char>((body_len >> 24) & 0xFF);
    buf[1] = static_cast<char>((body_len >> 16) & 0xFF);
    buf[2] = static_cast<char>((body_len >>  8) & 0xFF);
    buf[3] = static_cast<char>((body_len      ) & 0xFF);
}

/// Decode a 4-byte big-endian header into a body length.
inline std::uint32_t decode_header(const char* buf)
{
    return (static_cast<std::uint32_t>(static_cast<unsigned char>(buf[0])) << 24)
         | (static_cast<std::uint32_t>(static_cast<unsigned char>(buf[1])) << 16)
         | (static_cast<std::uint32_t>(static_cast<unsigned char>(buf[2])) <<  8)
         | (static_cast<std::uint32_t>(static_cast<unsigned char>(buf[3]))      );
}

// ── ChatMessage: a simple framed message buffer ────────────────────

class ChatMessage {
public:
    ChatMessage() : body_length_(0) {}

    /// Access the full buffer (header + body).
    const char* data() const { return data_.data(); }   // Read-only
    char*       data()       { return data_.data(); }   // Mutable for writing header + body

    std::size_t length() const { return HEADER_LENGTH + body_length_; }

    /// Access only the body portion.
    const char* body() const { return data_.data() + HEADER_LENGTH; }   // Read-only
    char*       body()       { return data_.data() + HEADER_LENGTH; }   // Mutable for writing body

    std::size_t body_length() const { return body_length_; }

    /// Set body length (call before writing into body()).
    void body_length(std::size_t new_length)
    {
        if (new_length > MAX_BODY_LENGTH)
            throw std::length_error("ChatMessage body exceeds MAX_BODY_LENGTH");
        body_length_ = new_length;
    }

    /// After reading the header bytes, call this to parse body_length_.
    bool decode_header()
    {
        body_length_ = harmony::decode_header(data_.data());
        if (body_length_ > MAX_BODY_LENGTH) {
            body_length_ = 0;
            return false;
        }
        return true;
    }

    /// Before sending, call this to stamp the header.
    void encode_header()
    {
        harmony::encode_header(data_.data(), static_cast<std::uint32_t>(body_length_));
    }

    /// Convenience: set body from a string.
    void set_body(const std::string& text)
    {
        body_length(text.size());
        std::memcpy(body(), text.data(), text.size());
        encode_header();
    }

    /// Convenience: extract body as a string.
    std::string body_as_string() const
    {
        return std::string(body(), body_length_);
    }

private:
    std::array<char, HEADER_LENGTH + MAX_BODY_LENGTH> data_{};
    std::size_t body_length_ = 0;
};

// ── Constants ──────────────────────────────────────────────────────

constexpr unsigned short DEFAULT_PORT = 9876;

} // namespace harmony

#endif // HARMONY_MESSAGE_HPP
