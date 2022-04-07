
#include "channel.hh"
#include "lock.hh"

Channel::Channel(char* debugName, Lock *senderLock, Lock *listenerLock) {
    name = debugName;

    sender = new Condition("sender_cond", senderLock)
    listener = new Condition("listener_cond", listenerLock)
}

Channel::~Channel() {
    delete sender;
    delete listener;
}

void Channel::Send(int msg) {
    if (s_busy) {

    } 
}

void Channel::Receive(int* msg) {

}