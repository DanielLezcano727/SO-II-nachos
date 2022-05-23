/// Simple program to test whether running a user program works.
///
/// Just do a “syscall” that shuts down the OS.
///
/// NOTE: for some reason, user programs with global data structures
/// sometimes have not worked in the Nachos environment.  So be careful out
/// there!  One option is to allocate data structures as automatics within a
/// procedure, but if you do this, you have to be careful to allocate a big
/// enough stack to hold the automatics!

#include "syscall.h"

int
main(void)
{
    Create("tet.txt");
    
    OpenFileId o = Open("tet.txt");
    Write("Hello world\n", 12, o);
    Close(o);

    char buffer[10];
    o = Open("tet.txt");
    Read(buffer, 4, CONSOLE_INPUT);
    Write(buffer, 4, CONSOLE_OUTPUT);
    Close(o);

    Remove("tet.txt");
    // Halt();
    return 0;
}
