#include "syscall.h"

#define ARGC_ERROR    "Error: missing argument."
#define MAX_LINE 256

int
main(int argc, char *argv[])
{
    if (argc != 2) {
        Write(ARGC_ERROR, sizeof(ARGC_ERROR) - 1, CONSOLE_OUTPUT);
        Exit(1);
    }

    int success = 1;
    OpenFileId file = Open(argv[1]); // Chequear si el primer argumento es el propio archivo
    Remove(argv[1]);
    Create(argv[1]); // Chequear si el primer argumento es el propio archivo
    OpenFileId newFile = Open(argv[1]); // Chequear si el primer argumento es el propio archivo
    char line[MAX_LINE];
    int size = 0;

    do {
        size = Read(line, MAX_LINE, file);
        if (size > 0) {
            Write(line, size, newFile);
        }
    } while (size > 0);

    Close(file);
    Close(newFile);

    return 0;
}
