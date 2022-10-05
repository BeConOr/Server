#pragma once

#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <string>

/**
 * @brief Class that stores and convert messages.
 */
class Message{
public:
    static constexpr std::size_t bodyLengthSize = 4;
    static constexpr std::size_t headerSize = bodyLengthSize;
    static constexpr std::size_t bodyMaxSize = 9999;

    Message()
        :mBodySize(0)
    {}

private:
    char mData[headerSize + bodyMaxSize];
    std::size_t mBodySize;
};