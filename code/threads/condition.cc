/// Routines for synchronizing threads.
///
/// The implementation for this primitive does not come with base Nachos.
/// It is left to the student.
///
/// When implementing this module, keep in mind that any implementation of a
/// synchronization routine needs some primitive atomic operation.  The
/// semaphore implementation, for example, disables interrupts in order to
/// achieve this; another way could be leveraging an already existing
/// primitive.
///
/// Copyright (c) 1992-1993 The Regents of the University of California.
///               2016-2021 Docentes de la Universidad Nacional de Rosario.
/// All rights reserved.  See `copyright.h` for copyright notice and
/// limitation of liability and disclaimer of warranty provisions.


#include "condition.hh"
#include "lock.hh"

/// Dummy functions -- so we can compile our later assignments.
///
/// Note -- without a correct implementation of `Condition::Wait`, the test
/// case in the network assignment will not work!

Condition::Condition(const char *debugName, Lock *conditionLock)
{
    name = debugName;
    lock = conditionLock;
    // lista = new List<>;
}

Condition::~Condition()
{
    // TODO
}

const char *
Condition::GetName() const
{
    return name;
}

void
Condition::Wait()
{
    // ASSERT(lock->isHeldByCurrentThread());
    // Semaphore *s = new Semaphore("s", 0);
    
    // lista->Append(s);
    // lock->Release();
    // s->P();
    // lock->Acquire();
    
    // delete s;
}

void
Condition::Signal()
{
    // ASSERT(lock->isHeldByCurrentThread());
    // if(!lista->isEmpty()){
    //     Semaphore *s = lista->Pop();

    //     if(s != NULL) {
    //         s->V();
    //     }
    // }
}

void
Condition::Broadcast()
{
    // TODO
}
