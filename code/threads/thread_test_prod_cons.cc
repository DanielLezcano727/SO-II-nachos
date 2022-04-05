/// Copyright (c) 1992-1993 The Regents of the University of California.
///               2007-2009 Universidad de Las Palmas de Gran Canaria.
///               2016-2021 Docentes de la Universidad Nacional de Rosario.
/// All rights reserved.  See `copyright.h` for copyright notice and
/// limitation of liability and disclaimer of warranty provisions.


#include "thread_test_prod_cons.hh"
#include "system.hh"
#include "condition.hh"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define END_AFTER 30
#define SHELF_SIZE 10
int shelf;
Lock *lock;
Condition *cond;

void
Producer()
{
    for(int i=0; i<END_AFTER; i++) {
        if(shelf<SHELF_SIZE) {
            lock->Acquire();
            shelf++;
            lock->Release();
            cond->Signal();
        }else {
            cond->Wait();
        }
    }
}

void
Consumer(void *name)
{
    while(true) {
        if(shelf>0) {
            lock->Acquire();
            shelf--;
            lock->Release();
            cond->Signal();
        }else {
            cond->Wait();
        }
    }
}

void
ThreadTestProdCons()
{
    lock = new Lock("cond_lock");
    cond = new Condition("cond", lock);

    char *name = new char [64];
    strncpy(name, "consumer", 64);
    Thread *newThread = new Thread(name);
    newThread->Fork(Consumer, (void *)name);
    Producer();
}
