#include "syscall.h"
#include "lib.c"

#define ARGC_ERROR    "Error: missing argument."

int
main(int argc, char *argv[])
{
    // Mkdir("foo");
    // Cd("foo");
    // Mkdir("bar");
    // Cd("foo/bar");
    // Mkdir("sad");
    // Ls();
    // Write(&"-\n", 2, CONSOLE_OUTPUT);
    // Cd("foo");
    // Ls();
    // Write(&"-\n", 2, CONSOLE_OUTPUT);
    // Cd("/");
    // Ls();
    if (argc != 2) {
        Write(ARGC_ERROR, sizeof(ARGC_ERROR) - 1, CONSOLE_OUTPUT);
        Exit(1);
    }

    Cd(argv[1]);

    return 0;
}