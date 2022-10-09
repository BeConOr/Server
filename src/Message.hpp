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

    const char * data() const
    {
        return mData;
    }

    char * data()
    {
        return mData;
    }

    std::size_t length() const
    {
        return headerSize + mBodySize;
    }

    const char * body() const
    {
        return mData + headerSize;
    }

    char * body()
    {
        return mData + headerSize;
    }

    std::size_t body_length() const
    {
        return mBodySize;
    }

    void body_length(std::size_t new_length)
    {
        mBodySize = new_length;
        if (mBodySize > bodyMaxSize)
            mBodySize = bodyMaxSize;
    }

    bool decode_header()
    {
        char body_size[bodyLengthSize + 1] = "";
        std::strncat(body_size, mData, bodyLengthSize);
        try
        {
            mBodySize = std::stoul(body_size);
        }
        catch (const std::exception & ex)
        {
            return false;
        }

        if (mBodySize > bodyMaxSize)
        {
            mBodySize = 0;
            return false;
        }

        return true;
    }

    void encode_header()
    {
        char body_length[bodyLengthSize + 1] = "";

        std::sprintf(body_length, "%4d", static_cast<int>(mBodySize));
        std::memcpy(mData, body_length, bodyLengthSize);
    }

    void clearData(){
        std::memset(mData, 0, headerSize + bodyMaxSize);
    }

private:
    char mData[headerSize + bodyMaxSize];
    std::size_t mBodySize;
};