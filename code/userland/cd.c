#include "syscall.h"

int
main(int argc, char *argv[])
{
//    Write(&"-\n", 2, CONSOLE_OUTPUT);
    Mkdir("foo");
    Cd("foo");
    Mkdir("bar");
    Cd("foo/bar");
    Mkdir("sad");
    Ls();
    Write(&"-\n", 2, CONSOLE_OUTPUT);
    Cd("foo");
    Ls();

    return 0;
}