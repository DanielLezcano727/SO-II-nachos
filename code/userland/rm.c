#include "syscall.h"

#define ARGC_ERROR    "Error: missing argument."

int
main(int argc, char *argv[])
{
    if (argc != 2) {
        Write(ARGC_ERROR, sizeof(ARGC_ERROR) - 1, CONSOLE_OUTPUT);
        Exit(1);
    }

    for (int i=1; i<argc; i++) {
        Remove(argv[i]);
    }
    return 0;
}
