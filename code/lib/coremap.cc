#include "coremap.hh"
#include <stdlib.h>

Coremap::Coremap(unsigned nitems): Bitmap(nitems) {
    vPage = new unsigned[nitems];
    thread = new int[nitems];
    referenced = new bool[nitems];
    for (unsigned i=0; i<nitems; i++)
        referenced[i] = false;
    victims = new List<int>;
}

Coremap::~Coremap() {
    delete[] vPage;
    delete[] thread;
    delete[] referenced;
    delete victims;
}

int Coremap::PickVictim() {
    int victim;

    #if defined(PRPOLICY_FIFO)
        victim = victims->Pop();
    #elif defined(PRPOLICY_SECOND_CHANCE)
        victim = victims->Pop();
        while (referenced[victim]) {
            referenced[victim] = false;
            victims->Append(victim);
            victim = victims->Pop();
        }
    #else
        victim = rand() % numBits;
    #endif
    return victim;
}

unsigned Coremap::GetThread(int index) {
    return thread[index]; // No se si habria que dividirlo por BITS_IN_WORD
}

int Coremap::GetVPN(int index) {
    return vPage[index];
}

void Coremap::Mark(unsigned which, unsigned vpn, int sid) {
    Bitmap::Mark(which);
    vPage[which] = vpn;
    thread[which] = sid;
    #if defined(PRPOLICY_FIFO) || defined(PRPOLICY_SECOND_CHANCE)
        victims->Append(which);
    #endif
}
