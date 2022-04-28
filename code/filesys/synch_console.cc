#include "synch_console.hh"

void RequestReadDone(void *arg) {
    ASSERT(arg != nullptr);
    SynchConsole *console = (SynchConsole *) arg;
    console->ReadDone();
}

void RequestWriteDone(void *arg) {
    ASSERT(arg != nullptr);
    SynchConsole *console = (SynchConsole *) arg;
    console->WriteDone();
}

SynchConsole::SynchConsole(const char *readFile, const char *writeFile) {

    semaphore_read = new Semaphore("synch console", 0);
    semaphore_write = new Semaphore("synch console", 0);
    lock_read = new Lock("synch console lock");
    lock_write = new Lock("synch console lock");
    console = new Console(readFile, writeFile, RequestReadDone, RequestWriteDone, this);
}

SynchConsole::~SynchConsole() {
    delete semaphore_read;
    delete semaphore_write;
    delete lock_read;
    delete lock_write;
    delete console;
}

void SynchConsole::PutChar(char ch) {
    lock_write->Acquire();
    console->PutChar(ch);
    semaphore_write->P();
    lock_write->Release();
}

char SynchConsole::GetChar() {
    lock_read->Acquire();
    char ch = console->GetChar();
    semaphore_read->P();
    lock_read->Release();
    return ch;
}

void SynchConsole::ReadDone() {
    semaphore_read->V();
}
void SynchConsole::WriteDone() {
    semaphore_write->V();
} 