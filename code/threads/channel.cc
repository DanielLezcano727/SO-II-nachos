
#include "channel.hh"
#include "lock.hh"

Channel::Channel(const char* debugName, Lock *senderLock, Lock *listenerLock) {
    name = debugName;
    s_busy = false;
    l_busy = false;
    lock_listener = new Lock("Listener lock");
    lock_sender = new Lock("Sender lock");
    sender = new Condition("sender_cond", senderLock);
    listener = new Condition("listener_cond", listenerLock);
}

Channel::~Channel() {
    delete sender;
    delete listener;
}

void Channel::Send(int msg) {
    lock_sender->Acquire();
    if (!l_busy) {
        s_busy = true;
        sender->Wait();
    }

    buffer = msg;
    s_busy = false;

    listener->Signal();
    sender->Wait();

    lock_sender->Release();
}

void Channel::Receive(int* msg) {
    lock_listener->Acquire();
    l_busy = true;
    if (s_busy) {
        sender->Signal();
    }

    listener->Wait();
    *msg = buffer;
    l_busy = false;

    sender->Signal();
    lock_listener->Release();
}