/// Data structures for the thread dispatcher and scheduler.
///
/// Primarily, the list of threads that are ready to run.
///
/// Copyright (c) 1992-1993 The Regents of the University of California.
///               2016-2021 Docentes de la Universidad Nacional de Rosario.
/// All rights reserved.  See `copyright.h` for copyright notice and
/// limitation of liability and disclaimer of warranty provisions.

#ifndef NACHOS_THREADS_CHANNEL__HH
#define NACHOS_THREADS_CHANNEL__HH

#include "thread.hh"
#include "condition.hh"

class Channel {
public:

    /// Initialize channel.
    Channel(char* debugName, Lock *senderLock, Lock *listenerLock);

    /// Delete channel.
    ~Channel();

    void Send(int msg);

    void Receive(int* msg);

private:
    int buffer;
    char *name;
    Condition *sender;
    bool s_busy;
    Condition *listener;
    bool l_busy;
};


#endif
