#pragma once
#include "../include/Protocol.hpp"

#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <cstdlib>
#include "Message.hpp"
#include <memory>
#include <vector>
#include <set>
#include <algorithm>
#include <thread>

namespace server {

    class Participant {
    public:
        virtual ~Participant() = default;

        virtual void deliver(const Message &message) = 0;

        virtual std::size_t getId() const = 0;
    };

    using ParticipantPtr = std::shared_ptr<Participant>;

    class Room {
    public:
        void join(const ParticipantPtr &participant) {
            mParticipants.insert(participant);
        }

        void leave(const ParticipantPtr &participant) {
            mParticipants.erase(participant);
        }

        void deliver(const std::size_t &id, const Message &message) {
            auto participantIter =
                    std::find_if(mParticipants.begin(),
                                 mParticipants.end(),
                                 [&id](ParticipantPtr current) { return current->getId() == id; });

            if (mParticipants.end() == participantIter)
                return;

            (*participantIter)->deliver(message);
        }

    private:
        std::set<ParticipantPtr> mParticipants;
    };

    class Session
            : public Participant
            , public std::enable_shared_from_this<Session> {
    public:
        Session(int socket, Room &room, std::size_t userId)
                : mSocket(socket), mRoom(room), mId(userId) {}

        void start() {
            mRoom.join(shared_from_this());
            while(true){
                doReadHeader();
            }
        }

        void deliver(const Message &msg) {
            doWrite(msg);
        }


        std::size_t getId() const override {
            return mId;
        }

    private:
        void doReadHeader() {
            auto self(shared_from_this());
            int ec = read(mSocket, mReadMessage.data(), Message::headerSize);
            if (ec != -1) {
                doReadBody();
            } else {
                mRoom.leave(shared_from_this());
            }
        }

        void doReadBody() {
            auto self(shared_from_this());
            int ec = read(mSocket, mReadMessage.body(), mReadMessage.body_length());

            if (ec != -1) {
                mRoom.deliver(mId, mReadMessage);
                std::cout << mReadMessage.body() << std::endl;
            } else {
                mRoom.leave(shared_from_this());
            }
        }

        void doWrite(const Message &message) {
            auto self(shared_from_this());
            int ec = write(mSocket, message.data(), message.length());
            if (ec == -1) {
                mRoom.leave(shared_from_this());
            }
        }


        int mSocket;
        Room mRoom;
        Message mReadMessage;
        const std::size_t mId;
    };

    class ServerPart {
    public:
        ServerPart(int connectionSocket)
                : mConnectionSocket(connectionSocket), mRoom(), mThreads(), mIdCounter(0)
        {}

        void start(){
            int ret = listen(mConnectionSocket, 20);
            if (ret == -1) {
                exit(EXIT_FAILURE);
            }
            while(true){
                doAccept();
            }
        }

    private:
        void makeSession(int socket, Room &room, std::size_t id) {
            std::make_shared<Session>(socket, room, id)->start();
        }

        void doAccept() {
            int dataSocket = accept(mConnectionSocket, NULL, NULL);
            if (dataSocket != -1) {
                mThreads.push_back(
                        std::thread(&ServerPart::makeSession, this, dataSocket, std::ref(mRoom), mIdCounter));
                ++mIdCounter;
            }
        }

        int mConnectionSocket;
        Room mRoom;
        std::size_t mIdCounter;
        std::vector<std::thread> mThreads;
    };
} //namespace server