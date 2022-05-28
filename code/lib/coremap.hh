#ifndef NACHOS_LIB_COREMAP__HH
#define NACHOS_LIB_COREMAP__HH

#include "bitmap.hh"

class Coremap: public Bitmap {
public:
    Coremap(unsigned nitems);
    ~Coremap();

    int PickVictim();
private:
    unsigned *vPage;
    int *thread; // int represents SpaceId
};

#endif