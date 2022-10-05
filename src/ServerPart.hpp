#pragma once
#include "../include/Protocol.hpp"

#include <sys/socket.h>
#include <sys/un.h>
#include "Message.hpp"
#include <memory>
#include <set>
#include <algorithm>

class Participant{
public:
    virtual ~Participant() = default;
    virtual void deliver(const Message & message) = 0;
    virtual std::size_t getId() const = 0;
};

using ParticipantPtr = std::shared_ptr<Participant>;

class Room{
public:
    void join(ParticipantPtr & participant){
        mParticipants.insert(participant);
    }

    void leave(ParticipantPtr & participant){
        mParticipants.erase(participant);
    }

    void deliver(const std::size_t & id, const Message & message){
        auto participantIter =
                std::find_if(mParticipants.begin(),
                             mParticipants.end(),
                             [&id](ParticipantPtr current){return current->getId() == id;});

        if(mParticipants.end() == participantIter)
            return;

        (*participantIter)->deliver(message);
    }

private:
    std::set<ParticipantPtr> mParticipants;
};

class Session
        : public Participant
        , public std::enable_shared_from_this<Session>
{
public:
    Session(int & socket, Room & room, std::size_t & userId)
        : mSocket(socket)
        , mRoom(room)
        , mId(userId)
    {}

    void start(){
        mRoom.join(shared_from_this());
    }

    std::size_t getId() const override{
        return mId;
    }

private:
    void doWrite(){

    }

    int mSocket;
    Room mRoom;
    Message mReadMessage;
    const std::size_t mId;
};