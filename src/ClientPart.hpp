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
    class ClientPart {
    public:
        using input_callback_type = std::function<void(Message)>;
        using interruption_callback_type = std::function<void(std::string)>;

        ClientPart(int socket)
                : mSocket(socket){
            memset(&mAddr, 0, sizeof(mAddr));
        }

        bool connect() {
            mAddr.sun_family = AF_UNIX;
            strncpy(mAddr.sun_path, gSocketName, sizeof(mAddr.sun_path) - 1);

            int ret = ::connect(mSocket, (const struct sockaddr *) &mAddr, sizeof(mAddr));
            if (ret == -1) {
                exit(EXIT_FAILURE);
            }
        }

        bool write(const Message & msg) {
            int ret = ::write(mSocket, msg.data(), msg.length() + 1);

            if (ret == -1) {
                closeWithError("Failed to write message");
                return false;
            }

            return true;
        }

        void close() {
            ::close(mSocket);
        }

        void start() {
            std::thread th(&ClientPart::doReadHeader, this);
            th.detach();
            while(true) {
                std::string message("");
                std::cin >> message;
                write(makeMessage(message));
            }
        }

    private:

        void doReadHeader() {
            while(true) {
                int ec = ::read(mSocket, mReadMessage.data(), Message::headerSize);
                if ((ec != -1) && mReadMessage.decode_header()) {
                    doReadBody();
                } else {
                    std::string error = "Failed to decode message";
                    closeWithError(error);
                }
            }
        }

        void doReadBody() {
            int ec = read(mSocket, mReadMessage.body(), mReadMessage.body_length());

            if (ec != -1)
            {
                //input_callback_(mReadMessage);
                std::cout << mReadMessage.data() << std::endl;

            }
            else
            {
                std::string error = "Failed to read message's body";
                closeWithError(error);
            }
        }

        void closeWithError(std::string error) {
            close();

            //interruption_callback_(error);
            std::cout << "ERROR: " << error << std::endl;
        }

    private:
        int mSocket;
        struct sockaddr_un mAddr;

//        input_callback_type input_callback_;
//        interruption_callback_type interruption_callback_;

        Message mReadMessage;
    };
}// namespace server