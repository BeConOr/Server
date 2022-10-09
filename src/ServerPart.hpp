#pragma once
#include "../include/Protocol.hpp"

#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <cstdlib>
#include "Message.hpp"
#include "MessageMaker.hpp"
#include <memory>
#include <vector>
#include <set>
#include <algorithm>
#include <thread>
#include <regex>

namespace server {

    /**
     * @brief Interface for participants that connects with server
     * */
    class Participant {
    public:
        virtual ~Participant() = default;

        virtual void deliver(const Message &message) = 0;

        virtual std::size_t getId() const = 0;
    };

    using ParticipantPtr = std::shared_ptr<Participant>;


    /**
     * @brief Class that stores connections information
     * */
    class Room {
    public:
        /**
         * @brief Add a new participant to server
         * */
        void join(const ParticipantPtr &participant) {
            mParticipants.insert(participant);
        }

        /**
         * @brief Remove a participant from server
         * */
        void leave(const ParticipantPtr &participant) {
            mParticipants.erase(participant);
        }

        /**
         * @brief Function to send message to participant
         * @param id - participant's id who you want to send message to
         * @param message - message to send
         * */
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


    /**
     * @brief Class to manage current session.
     * */
    class Session
            : public Participant
            , public std::enable_shared_from_this<Session> {
    public:
        Session(int socket, Room &room, std::size_t userId)
                : mSocket(socket), mRoom(room), mId(userId) {}

        /**
         * @brief Function to start session
         * */
        void start() {
            mRoom.join(shared_from_this());
            while(true){
                doReadHeader();
            }
        }

        /**
         * @brief Function to send message to current participant
         * */
        void deliver(const Message &msg) {
            doWrite(msg);
        }


        /**
         * @brief Function to get id of current session
         * */
        std::size_t getId() const override {
            return mId;
        }

    private:
        void doReadHeader() {
            auto self(shared_from_this());
            mReadMessage.clearData();
            int ec = read(mSocket, mReadMessage.data(), Message::headerSize);
            if ((ec != -1)) {
                if(mReadMessage.decode_header()) doReadBody();
            } else {
                mRoom.leave(shared_from_this());
            }
        }

        void doReadBody() {
            auto self(shared_from_this());
            int ec = read(mSocket, mReadMessage.body(), mReadMessage.body_length());

            if (ec != -1) {
                std::string command(mReadMessage.body());
                if(command.find(commands::gStartMeasure) == 0){
                    std::vector<int> channelNumbers(findChannels(command));
                    for(auto number : channelNumbers){
                        std::cout << number << std::endl;
                    }
                    if(!channelNumbers.empty()) {
                        mRoom.deliver(mId, makeMessage("ok"));
                    }else{
                        mRoom.deliver(mId, makeMessage("fail"));
                    }
                }else if(command.find(commands::gSetRange) == 0){
                    std::regex rangeWords("channel(\\d+), range(\\d+)");
                    std::smatch sm;
                    std::string result("");
                    if(std::regex_search(command, sm, rangeWords)) {
                        for (auto number : sm) {
                            std::cout << number << '\n';
                        }
                        if(std::stoi(sm[2]) < 4) {
                            result.append("ok, ");
                        }else{
                            result.append("fail, ");
                        }
                        result.append("range");
                        result.append(sm[2]);
                    }else{
                        result.append("fail");
                    }
                    mRoom.deliver(mId, makeMessage(result));
                }else if(command.find(commands::gStopMeasure) == 0){
                    std::vector<int> channelNumbers(findChannels(command));
                    for(auto number : channelNumbers){
                        std::cout << number << std::endl;
                    }
                    if(!channelNumbers.empty()) {
                        mRoom.deliver(mId, makeMessage("ok"));
                    }else{
                        mRoom.deliver(mId, makeMessage("fail"));
                    }
                }else if(command.find(commands::gGetStatus) == 0){
                    std::vector<int> channelNumbers(findChannels(command));
                    for(auto number : channelNumbers){
                        std::cout << number << std::endl;
                    }
                    if(!channelNumbers.empty()) {
                        mRoom.deliver(mId, makeMessage("ok"));
                    }else{
                        mRoom.deliver(mId, makeMessage("fail"));
                    }
                }else if(command.find(commands::gGetResult) == 0){
                    std::vector<int> channelNumbers(findChannels(command));
                    for(auto number : channelNumbers){
                        std::cout << number << std::endl;
                    }
                    if(!channelNumbers.empty()) {
                        mRoom.deliver(mId, makeMessage("ok"));
                    }else{
                        mRoom.deliver(mId, makeMessage("fail"));
                    }
                }
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

        std::vector<int> findChannels(std::string & command){
            std::vector<int> channelsNumber = std::vector<int>();
            std::regex rangeWords("channel(\\d+)");
            auto words_begin =
                    std::sregex_iterator(command.begin(), command.end(), rangeWords);
            auto words_end = std::sregex_iterator();
            for (std::sregex_iterator i = words_begin; i != words_end; ++i) {
                std::smatch match = *i;
                channelsNumber.push_back(std::stoi(match[1]));
            }
            return channelsNumber;
        }


        int mSocket;
        Room mRoom;
        Message mReadMessage;
        const std::size_t mId;
    };


    /**
     * @brief Class that manage server
     * */
    class ServerPart {
    public:
        ServerPart(int connectionSocket)
                : mConnectionSocket(connectionSocket), mRoom(), mThreads(), mIdCounter(0)
        {}

        /**
         * @brief Function to start server
         * */
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
        /**
         * @brief Function to create a thread for new connection
         * @param socket - socket of new connection
         * @param room - room that store data of server
         * @param id - id of new connection
         * */
        void makeSession(int socket, Room &room, std::size_t id) {
            std::make_shared<Session>(socket, room, id)->start();
        }

        /**
         * @brief Function to create a new connection
         * */
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