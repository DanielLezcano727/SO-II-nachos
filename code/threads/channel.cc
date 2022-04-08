
#include "channel.hh"
#include "lock.hh"

Channel::Channel(const char* debugName) {
    name = debugName;
    s_busy = false;
    l_busy = false;

    lock_listener = new Lock("Listener lock");
    lock_sender = new Lock("Sender lock");

    cond_listener_lock = new Lock("cond_listener_lock");
    cond_sender_lock = new Lock("cond_sender_lock");
    sender = new Condition("sender_cond", cond_sender_lock);
    listener = new Condition("listener_cond", cond_listener_lock);
}

Channel::~Channel() {
    delete sender;
    delete listener;
    delete lock_sender;
    delete lock_listener;
    delete cond_sender_lock;
    delete cond_listener_lock;

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