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

#define SHELF_SIZE 256
#define TO_PRODUCE 10
#define MAX_PRODUCE 3
#define MAX_CONS 2
List<int> *shelf;
Lock *lock;
Condition *cond;

void
Producer(int ammount)
{
    int step;

    for(int i=0; i<ammount; i++) {
        step = rand() % MAX_PRODUCE;
        for(int j=0; j<step; j++) {
            shelf->Append(j);
        }
        cond->Signal();
        cond->Wait();
    }
}

void
Consumer(void *name)
{
    int step;

    while(true) {
        cond->Wait();
        step = rand() % MAX_CONS;
        for(int j=0; j<step; j++) {
            shelf->Pop();
        }
        cond->Signal();
    }
}

void
ThreadTestProdCons()
{
    shelf = new List<int>;
    lock = new Lock("cond_lock");
    cond = new Condition("cond", lock);

    char *name = new char [64];
    strncpy(name, "consumer", 64);
    Thread *newThread = new Thread(name);
    newThread->Fork(Consumer, (void *)name);

    lock->Acquire();
    Producer(TO_PRODUCE);
}
