#ifndef NACHOS_LIB_COREMAP__HH
#define NACHOS_LIB_COREMAP__HH

#include "bitmap.hh"
#include "list.hh"

class Coremap: public Bitmap {
public:
    Coremap(unsigned nitems);
    ~Coremap();

    unsigned GetThread(int index);
    int GetVPN(int index);

    int PickVictim();

    void Mark(unsigned which, unsigned vpn, int sid);

private:
    List<int> *victims;
    bool *referenced;
    unsigned *vPage;
    int *thread; // int represents SpaceId
};

#endif