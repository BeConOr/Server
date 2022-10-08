#pragma once


#include "Message.hpp"

#include <string_view>

Message makeMessage(std::string_view data = "");

