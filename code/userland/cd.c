#include "syscall.h"
#include "lib.c"

int
main(int argc, char *argv[])
{
    Mkdir("foo");
    Cd("foo");
    Mkdir("bar");
    Cd("foo/bar");
    Mkdir("sad");
    Ls();
    Cd("foo");
    Ls();
    Cd("/");
    Ls();

    return 0;
}