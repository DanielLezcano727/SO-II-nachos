/// Entry points into the Nachos kernel from user programs.
///
/// There are two kinds of things that can cause control to transfer back to
/// here from user code:
///
/// * System calls: the user code explicitly requests to call a procedure in
///   the Nachos kernel.  Right now, the only function we support is `Halt`.
///
/// * Exceptions: the user code does something that the CPU cannot handle.
///   For instance, accessing memory that does not exist, arithmetic errors,
///   etc.
///
/// Interrupts (which can also cause control to transfer from user code into
/// the Nachos kernel) are handled elsewhere.
///
/// For now, this only handles the `Halt` system call.  Everything else core-
/// dumps.
///
/// Copyright (c) 1992-1993 The Regents of the University of California.
///               2016-2021 Docentes de la Universidad Nacional de Rosario.
/// All rights reserved.  See `copyright.h` for copyright notice and
/// limitation of liability and disclaimer of warranty provisions.


#include "transfer.hh"
#include "syscall.h"
#include "filesys/open_file.hh"
#include "filesys/directory_entry.hh"
#include "threads/system.hh"

#include <stdio.h>

#define FILE_SIZE 256

static void
IncrementPC()
{
    unsigned pc;

    pc = machine->ReadRegister(PC_REG);
    machine->WriteRegister(PREV_PC_REG, pc);
    pc = machine->ReadRegister(NEXT_PC_REG);
    machine->WriteRegister(PC_REG, pc);
    pc += 4;
    machine->WriteRegister(NEXT_PC_REG, pc);
}

/// Do some default behavior for an unexpected exception.
///
/// NOTE: this function is meant specifically for unexpected exceptions.  If
/// you implement a new behavior for some exception, do not extend this
/// function: assign a new handler instead.
///
/// * `et` is the kind of exception.  The list of possible exceptions is in
///   `machine/exception_type.hh`.
static void
DefaultHandler(ExceptionType et)
{
    int exceptionArg = machine->ReadRegister(2);

    fprintf(stderr, "Unexpected user mode exception: %s, arg %d.\n",
            ExceptionTypeToString(et), exceptionArg);
    ASSERT(false);
}

/// Handle a system call exception.
///
/// * `et` is the kind of exception.  The list of possible exceptions is in
///   `machine/exception_type.hh`.
///
/// The calling convention is the following:
///
/// * system call identifier in `r2`;
/// * 1st argument in `r4`;
/// * 2nd argument in `r5`;
/// * 3rd argument in `r6`;
/// * 4th argument in `r7`;
/// * the result of the system call, if any, must be put back into `r2`.
///
/// And do not forget to increment the program counter before returning. (Or
/// else you will loop making the same system call forever!)
static void
SyscallHandler(ExceptionType _et)
{
    int scid = machine->ReadRegister(2);

    switch (scid) {

        case SC_HALT:
            DEBUG('e', "Shutdown, initiated by user program.\n");
            interrupt->Halt();
            break;

        case SC_CREATE: {
            int filenameAddr = machine->ReadRegister(4);
            if (filenameAddr == 0) {
                DEBUG('e', "Error: address to filename string is null.\n");
            }

            char filename[FILE_NAME_MAX_LEN + 1];
            if (!ReadStringFromUser(filenameAddr,
                                    filename, sizeof filename)) {
                DEBUG('e', "Error: filename string too long (maximum is %u bytes).\n",
                      FILE_NAME_MAX_LEN);
            }

            if (!fileSystem->Create(filename, FILE_SIZE)) {
                DEBUG('e', "Error: couldn't create");
            }

            DEBUG('e', "`Create` requested for file `%s`.\n", filename);
            break;
        }

        case SC_REMOVE: {
            int filenameAddr = machine->ReadRegister(4);
            if (filenameAddr == 0) {
                DEBUG('e', "Error: address to filename string is null.\n");
            }

            char filename[FILE_NAME_MAX_LEN + 1];
            if (!ReadStringFromUser(filenameAddr,
                                    filename, sizeof filename)) {
                DEBUG('e', "Error: filename string too long (maximum is %u bytes).\n",
                      FILE_NAME_MAX_LEN);
            }

            if (!fileSystem->Remove(filename)) {
                DEBUG('e', "Error: couldn't remove, file doesn't exist");
            }

            DEBUG('e', "`Create` requested for file `%s`.\n", filename);
            break;
        }

        case SC_EXIT: {
            currentThread->Finish();

            break;
        }

        case SC_WRITE: {
            int stringAddr = machine->ReadRegister(4);
            int size = machine->ReadRegister(5);
            OpenFileId id = machine->ReadRegister(6);

            if (!stringAddr)
                DEBUG('e', "Error: address to string is null.\n");
            else if (size < 0)
                DEBUG('e', "Error: invalid string size.\n");
            else if (id < CONSOLE_OUTPUT)
                DEBUG('e', "Error: invalid file descriptor.\n");
            else {
                char str[size];

                ReadBufferFromUser(stringAddr, str, size);

                if (id == CONSOLE_OUTPUT) {
                    for (int i = 0; i < size; i++) {
                        synchConsole->PutChar(str[i]);
                    }
                }else if (currentThread->fileList->HasKey(id)) {
                    currentThread->fileList->Get(id)->Write(str, size);
                }else {
                    DEBUG('e', "Error: no file with that id");
                }
            }

            break;
        }

        case SC_READ: {
            int usrAddr = machine->ReadRegister(4);
            int size = machine->ReadRegister(5);
            OpenFileId id = machine->ReadRegister(6);

            int i;
            if (!usrAddr)
                DEBUG('e', "Error: address to usr is null.\n");
            else if (size < 0)
                DEBUG('e', "Error: invalid size.\n");
            else if (id <= CONSOLE_OUTPUT)
                DEBUG('e', "Error: invalid file descriptor.\n");
            else {
                char str[size];

                if (id == CONSOLE_INPUT) {
                    for(i = 0; i < size - 1; i++) {
                        str[i] = synchConsole->GetChar();
                    }
                    str[i] = '\0';
                }else if (currentThread->fileList->HasKey(id)) {
                    i = currentThread->fileList->Get(id)->Read(str, size);
                }else {
                    DEBUG('e', "Error: no file with that id");
                }

                WriteStringToUser(str, usrAddr);
            }
            
            machine->WriteRegister(2, i);
            break;
        }

        case SC_CLOSE: {
            int fid = machine->ReadRegister(4);
            OpenFile* opfid = (OpenFile*) fid;

            if (fid < 0)
                DEBUG('e',"Invalid file id");
            else if (currentThread->fileTable->HasKey(fid))
                currentThread->fileTable->Remove(fid);
            else {
                DEBUG('e', "Error: file descriptor not opened.\n");
            }
            break;
        }

        case SC_OPEN: {
            int filenameAddr = machine->ReadRegister(4);
            if (filenameAddr == 0) {
                DEBUG('e', "Error: address to filename is null.\n");
            }

            char filename[FILE_NAME_MAX_LEN + 1];
            if (!ReadStringFromUser(filenameAddr,
                                    filename, sizeof filename)) {
                DEBUG('e', "Error: filename string too long (maximum is %u bytes).\n",
                      FILE_NAME_MAX_LEN);
            }

            OpenFile* fid = fileSystem->Open(filename);
            currentThread->fileTable->Add(fid);
            DEBUG('e', "`Open` requested for id %u.\n", fid);

            machine->WriteRegister(2, fid);
            break;
        }

        case SC_JOIN: {
            break;
        }

        case SC_EXEC: {
            break;
        }
        
        case SC_PS: {
            scheduler->Print();
            break;
        }

        default:
            fprintf(stderr, "Unexpected system call: id %d.\n", scid);
            ASSERT(false);

    }

    IncrementPC();
}


/// By default, only system calls have their own handler.  All other
/// exception types are assigned the default handler.
void
SetExceptionHandlers()
{
    machine->SetHandler(NO_EXCEPTION,            &DefaultHandler);
    machine->SetHandler(SYSCALL_EXCEPTION,       &SyscallHandler);
    machine->SetHandler(PAGE_FAULT_EXCEPTION,    &DefaultHandler);
    machine->SetHandler(READ_ONLY_EXCEPTION,     &DefaultHandler);
    machine->SetHandler(BUS_ERROR_EXCEPTION,     &DefaultHandler);
    machine->SetHandler(ADDRESS_ERROR_EXCEPTION, &DefaultHandler);
    machine->SetHandler(OVERFLOW_EXCEPTION,      &DefaultHandler);
    machine->SetHandler(ILLEGAL_INSTR_EXCEPTION, &DefaultHandler);
}
