#include "coremap.hh"
#include <stdlib.h>

Coremap::Coremap(unsigned nitems): Bitmap(nitems) {
    vPage = new unsigned[nitems];
    thread = new int[nitems];
}

Coremap::~Coremap() {
    delete[] vPage;
    delete[] thread;
}

int Coremap::PickVictim() {
    return rand() % numWords;
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
}
