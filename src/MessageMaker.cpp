#include "MessageMaker.hpp"

Message makeMessage(std::string_view data)
{
    Message msg;

    msg.body_length(data.size());
    std::memcpy(msg.body(), data.data(), msg.body_length());
    msg.encode_header();
    return msg;
}
