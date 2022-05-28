#include "coremap.hh"
#include <stdlib.h>

Coremap::Coremap(unsigned nitems): Bitmap(nitems) {
    vPage = new unsigned[numWords];
    thread = new int[numWords];
}

Coremap::~Coremap() {
    delete[] vPage;
    delete[] thread;
}

int Coremap::PickVictim() {
    return rand() % numWords;
}
