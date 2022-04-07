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
#include <cstring>
#include <string>

#define PRODUCERS 6
#define CONSUMERS 6

#define END_AFTER 100 // Cantidad de iteraciones de cada productor
#define BUFFER_SIZE 10 // Cantidad maxima del buffer
int buffer;

// Descomentar los locks cuando haya más de un procesador
// Lock *lock; // Utilizado para proteger la variable buffer (Region critica)
Condition *cond_prod; // Utilizado para que los productores no produzcan mas de lo que se puede guardar en el buffer
Condition *cond_cons; // Utilizado para que los consumidores no consuman mas de lo que este almacenado en el buffer

void
Producer(void *name)
{
    DEBUG('t', "Iniciando Thread %s\n", name);
    for(int i=0; i<END_AFTER; i++) {
        if(buffer<BUFFER_SIZE) {
            DEBUG('t', "Thread %s produce\n", name);
            // lock->Acquire();
            buffer++;
            // lock->Release();
            DEBUG('t', "Thread %s despierta consumidores\n", name);
            cond_cons->Signal();
            currentThread->Yield();
        } else {
            DEBUG('t', "Buffer lleno. Thread %s a dormir\n", name);
            currentThread->Yield();
            cond_prod->Wait();
            DEBUG('t', "Buffer liberado. Thread %s sigue\n", name);
        }
    }
}

void
Consumer(void *name)
{
    while(true) {
        if(buffer>0) {
            DEBUG('t', "Thread %s consume\n", name);
            // lock->Acquire();
            buffer--;
            // lock->Release();
            DEBUG('t', "Thread %s despierta productores\n", name);
            cond_prod->Signal();
            currentThread->Yield();
        }else {
            DEBUG('t', "Buffer vacio. Thread %s a dormir\n", name);
            currentThread->Yield();
            cond_cons->Wait();
            DEBUG('t', "Buffer tiene para consumir. Thread %s sigue\n", name);
        }
    }
}

void
ThreadTestProdCons()
{
    lock = new Lock("Lock buffer");
    cond_cons = new Condition("cond", new Lock("cond_lock"));
    cond_prod = new Condition("cond", new Lock("prod_lock"));

    char *nameConsumers[CONSUMERS];
    Thread *consumers[CONSUMERS];

    std::string nameCons = "consumer ";
    std::string nameProd = "producer ";

    for (int i = 0; i<CONSUMERS; i++) {
        nameConsumers[i] = new char [64];
        
        strncpy(nameConsumers[i], (nameCons + std::to_string(i)).c_str(), 64);
        
        consumers[i] = new Thread(nameConsumers[i]);
        consumers[i]->Fork(Consumer, (void *)nameConsumers[i]);
    }

    char *nameProducers[PRODUCERS-1];
    Thread *producers[PRODUCERS-1];

    for (int i = 0; i<PRODUCERS-1; i++) {
        nameProducers[i] = new char [64];
        strncpy(nameProducers[i], (nameProd + std::to_string(i)).c_str(), 64);
        
        producers[i] = new Thread(nameProducers[i]);
        
        producers[i]->Fork(Producer, (void *)nameProducers[i]);
    }
    Producer("main Producer");
}
