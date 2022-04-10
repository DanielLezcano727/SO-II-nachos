
#include "thread_test_channels.hh"
#include "system.hh"
#include "channel.hh"

#include <stdio.h>
#include <string>
#include <cstring>

#define COMUNICADORES 3
#define RECEPTORES 3
#define END_AFTER 10

static Channel *channel;

static void
Comunicador(void *name)
{
    for (int i=0; i<END_AFTER; i++) {
        channel->Send(i);
        currentThread->Yield();
    }
}

static void
Receptor(void *name)
{
    int msg;
    for (int i=0; i<END_AFTER; i++) {
        channel->Receive(&msg);
        printf("%s recibio %d\n", (char*)name, msg);
        currentThread->Yield();
    }
}

void
ThreadTestChannels()
{
    channel = new Channel((char*)"channel");

    Thread *comunicadores[COMUNICADORES];
    Thread *receptores[RECEPTORES];
    std::string nameCom = "comunicador ";
    std::string nameRec = "receptor ";
    char *nameComunicadores[COMUNICADORES];
    char *nameReceptores[RECEPTORES];

    for(int i=0; i<COMUNICADORES; i++) {
        nameComunicadores[i] = new char [64];
        
        strncpy(nameComunicadores[i], (nameCom + std::to_string(i)).c_str(), 64);
        
        comunicadores[i] = new Thread(nameComunicadores[i]);
        comunicadores[i]->Fork(Comunicador, (void *)nameComunicadores[i]);
    }

    for(int j=0; j<RECEPTORES-1; j++) {
        nameReceptores[j] = new char [64];
        
        strncpy(nameReceptores[j], (nameRec + std::to_string(j)).c_str(), 64);
        
        receptores[j] = new Thread(nameReceptores[j]);
        receptores[j]->Fork(Receptor, (void *)nameReceptores[j]);
    }

    Receptor((void*)"main Receptor");
}