/// Copyright (c) 1992-1993 The Regents of the University of California.
///               2007-2009 Universidad de Las Palmas de Gran Canaria.
///               2016-2021 Docentes de la Universidad Nacional de Rosario.
/// All rights reserved.  See `copyright.h` for copyright notice and
/// limitation of liability and disclaimer of warranty provisions.


#include "thread_test_simple.hh"
#include "system.hh"
#include "semaphore.hh"

#include <stdio.h>
#include <string.h>

static Semaphore *sem;

/// Loop 10 times, yielding the CPU to another ready thread each iteration.
///
/// * `name` points to a string with a thread name, just for debugging
///   purposes.
void
SimpleThread(void *name_)
{
    // Reinterpret arg `name` as a string.
    char *name = (char *) name_;
    #ifdef SEMAPHORE_TEST
        sem->P();
        DEBUG('s', "Thread %s executes sem->P()\n", name);
    #endif
    // If the lines dealing with interrupts are commented, the code will
    // behave incorrectly, because printf execution may cause race
    // conditions.
    for (unsigned num = 0; num < 10; num++) {
        printf("*** Thread `%s` is running: iteration %u\n", name, num);
        currentThread->Yield();
    }

    #ifdef SEMAPHORE_TEST
        sem->V();
        DEBUG('s', "Thread %s executes sem->V()\n", name);
    #endif

    printf("!!! Thread `%s` has finished\n", name);
}

/// Set up a ping-pong between several threads.
///
/// Do it by launching one thread which calls `SimpleThread`, and finally
/// calling `SimpleThread` on the current thread.
void
ThreadTestSimple()
{
    char *name5 = new char [64];
    strncpy(name5, "5th", 64);
    Thread *newThread5 = new Thread(name5);

    char *name4 = new char [64];
    strncpy(name4, "4th", 64);
    Thread *newThread4 = new Thread(name4);
    
    char *name3 = new char [64];
    strncpy(name3, "3rd", 64);
    Thread *newThread3 = new Thread(name3);

    char *name2 = new char [64];
    strncpy(name2, "2nd", 64);
    Thread *newThread2 = new Thread(name2);

    #ifdef SEMAPHORE_TEST
        sem = new Semaphore("Sem", 3);
    #else
        sem = NULL;
    #endif

    newThread5->Fork(SimpleThread, (void *) name5);
    newThread4->Fork(SimpleThread, (void *) name4);
    newThread3->Fork(SimpleThread, (void *) name3);
    newThread2->Fork(SimpleThread, (void *) name2);
    SimpleThread((void *) "1st");

    #ifdef SEMAPHORE_TEST
        delete sem;
    #endif
}
