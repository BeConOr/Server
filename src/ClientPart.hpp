#pragma once

#include <cstdlib>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <errno.h>

#include <deque>
#include <iostream>
#include <thread>
#include <functional>

#include "../include/Protocol.hpp"

#include "Message.hpp"
#include "MessageMaker.hpp"

namespace server {
    /**
     * @brief Class to connect with server and send command.
     * */
    class ClientPart {
    public:

        ClientPart(int socket)
                : mSocket(socket){
            memset(&mAddr, 0, sizeof(mAddr));
        }

        /**
         * @brief Function to connect with server.
         * */
        bool connect() {
            mAddr.sun_family = AF_UNIX;
            strncpy(mAddr.sun_path, gSocketName, sizeof(mAddr.sun_path) - 1);

            int ret = ::connect(mSocket, (const struct sockaddr *) &mAddr, sizeof(mAddr));
            if (ret == -1) {
                exit(EXIT_FAILURE);
            }
        }

        /**
         * @brief Function to sent message to server.
         * */
        bool write(const Message & msg) {
            int ret = ::write(mSocket, msg.data(), msg.length() + 1);

            if (ret == -1) {
                closeWithError("Failed to write message");
                return false;
            }

            return true;
        }

        /**
         * @brief Function to close connection.
         * */
        void close() {
            ::close(mSocket);
        }

        /**
         * @brief Function to start client.
         * */
        void start() {
            std::thread th(&ClientPart::doReadHeader, this);
            th.detach();
            while(true) {
                std::string message;
                std::getline(std::cin, message);
                if(message == gEnd) break;
                write(makeMessage(message));
            }
        }

    private:

        /**
         * @brief Function to read headers of message from server (message length etc.)
         * */
        void doReadHeader() {
            while(true) {
                mReadMessage.clearData();
                int ec = ::read(mSocket, mReadMessage.data(), Message::headerSize);
                if ((ec != -1) && mReadMessage.decode_header()) {
                    doReadBody();
                } else {
                    std::string error = "Failed to decode message";
                    closeWithError(error);
                }
            }
        }

        /**
         * @brief Function to read body of message.
         * */
        void doReadBody() {
            int ec = read(mSocket, mReadMessage.body(), mReadMessage.body_length());

            if (ec != -1)
            {
                std::cout << mReadMessage.body() << std::endl;

            }
            else
            {
                std::string error = "Failed to read message's body";
                closeWithError(error);
            }
        }

        /**
         * @brief Function to close client when error occurs
         * */
        void closeWithError(std::string error) {
            close();

            std::cout << "ERROR: " << error << std::endl;
            exit(EXIT_FAILURE);
        }

    private:
        int mSocket;
        struct sockaddr_un mAddr;

        Message mReadMessage;
    };
}// namespace server