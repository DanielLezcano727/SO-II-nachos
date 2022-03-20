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


/// Loop 10 times, yielding the CPU to another ready thread each iteration.
///
/// * `name` points to a string with a thread name, just for debugging
///   purposes.
void
SimpleThread(void *name_)
{
    // Reinterpret arg `name` as a string.
    char *name = (char *) name_;

    // If the lines dealing with interrupts are commented, the code will
    // behave incorrectly, because printf execution may cause race
    // conditions.
    for (unsigned num = 0; num < 10; num++) {
        printf("*** Thread `%s` is running: iteration %u\n", name, num);
        currentThread->Yield();
    }
    printf("!!! Thread `%s` has finished\n", name);
}

void
SimpleThread(void *name_, Semaphore *sem)
{
    // Reinterpret arg `name` as a string.
    char *name = (char *) name_;

    sem->P();
    printf("*** Thread %s realizo un P\n", name);
    // If the lines dealing with interrupts are commented, the code will
    // behave incorrectly, because printf execution may cause race
    // conditions.
    for (unsigned num = 0; num < 10; num++) {
        printf("*** Thread `%s` is running: iteration %u\n", name, num);
        currentThread->Yield();
    }
    printf("!!! Thread `%s` has finished\n", name);
    sem->V();
    printf("*** Thread %s realizo un V\n", name);
}

/// Set up a ping-pong between several threads.
///
/// Do it by launching one thread which calls `SimpleThread`, and finally
/// calling `SimpleThread` on the current thread.
void
ThreadTestSimple()
{
    char *name = new char [64];

    strncpy(name, "5th", 64);
    Thread *newThread5 = new Thread(name);

    strncpy(name, "4th", 64);
    Thread *newThread4 = new Thread(name);
    
    strncpy(name, "3rd", 64);
    Thread *newThread3 = new Thread(name);

    strncpy(name, "2nd", 64);
    Thread *newThread2 = new Thread(name);

    #ifdef SEMAPHORE_TEST
        printf("Pija\n");
        strncpy(name, "Sem", 64);
        Semaphore *sem = new Semaphore(name, 3)
        
        newThread5->Fork(SimpleThread, (void *) name, sem);
        newThread4->Fork(SimpleThread, (void *) name, sem);
        newThread3->Fork(SimpleThread, (void *) name, sem);
        newThread2->Fork(SimpleThread, (void *) name, sem);
        SimpleThread((void *) "1st", sem);
        
        sem->~Semaphore();
    #else
        newThread5->Fork(SimpleThread, (void *) name);
        newThread4->Fork(SimpleThread, (void *) name);
        newThread3->Fork(SimpleThread, (void *) name);
        newThread2->Fork(SimpleThread, (void *) name);
        SimpleThread((void *) "1st");
    #endif
}
