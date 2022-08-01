#include "syscall.h"

int
main(int argc, char *argv[])
{
    Ls();
    Write(&"-\n", 2, CONSOLE_OUTPUT);
    Mkdir("foo");
    Cd("foo");
    Ls();
    Write(&"a\n", 2, CONSOLE_OUTPUT);
    Mkdir("bar");
    Ls();

    return 0;
}