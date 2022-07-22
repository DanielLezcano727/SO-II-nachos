/// Routines for managing the disk file header (in UNIX, this would be called
/// the i-node).
///
/// The file header is used to locate where on disk the file's data is
/// stored.  We implement this as a fixed size table of pointers -- each
/// entry in the table points to the disk sector containing that portion of
/// the file data (in other words, there are no indirect or doubly indirect
/// blocks). The table size is chosen so that the file header will be just
/// big enough to fit in one disk sector,
///
/// Unlike in a real system, we do not keep track of file permissions,
/// ownership, last modification date, etc., in the file header.
///
/// A file header can be initialized in two ways:
///
/// * for a new file, by modifying the in-memory data structure to point to
///   the newly allocated data blocks;
/// * for a file already on disk, by reading the file header from disk.
///
/// Copyright (c) 1992-1993 The Regents of the University of California.
///               2016-2021 Docentes de la Universidad Nacional de Rosario.
/// All rights reserved.  See `copyright.h` for copyright notice and
/// limitation of liability and disclaimer of warranty provisions.


#include "file_header.hh"
#include "threads/system.hh"

#include <ctype.h>
#include <stdio.h>

static const unsigned FREE_MAP_SECTOR = 0;

bool 
FileHeader::Expand(unsigned numBytes) {
    bool res = true;
    if(raw.nextHeader != -1) {
        FileHeader* header = new FileHeader;
        header->FetchFrom(raw.nextHeader);
        res = header->Expand(numBytes);
        header->WriteBack(raw.nextHeader);
        delete header;
    }else {
        OpenFile* freeMapFile   = new OpenFile(FREE_MAP_SECTOR); //Podemos abrir de nuevo el freeMapFile (???)
        Bitmap* freeMap = new Bitmap(NUM_SECTORS);
        freeMap->FetchFrom(freeMapFile);
        unsigned newNumBytes = numBytes - (SECTOR_SIZE - raw.numBytes); //Take into account left-over bytes in last sector
        
        if(newNumBytes <= 0) {
            raw.numBytes += numBytes;
            return true;
        }
        if (freeMap->CountClear() < DivRoundUp(numBytes, SECTOR_SIZE)) {
            return false;  // Not enough space.
        }

        unsigned int oldnumSectors = raw.numSectors;
        raw.numBytes = raw.numBytes + numBytes < MAX_FILE_SIZE ? raw.numBytes + numBytes : MAX_FILE_SIZE;
        const unsigned int sectores = DivRoundUp(newNumBytes, SECTOR_SIZE);
        raw.numSectors = sectores < NUM_DIRECT ? sectores : NUM_DIRECT;

        for (unsigned i = oldnumSectors + 1; i < raw.numSectors; i++) {
            raw.dataSectors[i] = freeMap->Find();
        }

        if (sectores > raw.numSectors - oldnumSectors) {
            raw.nextHeader = freeMap->Find();
            if (raw.nextHeader == -1)
                return false;

            FileHeader* header = new FileHeader;
            res = header->Allocate(freeMap, numBytes - (NUM_DIRECT - (oldnumSectors + 1)) * SECTOR_SIZE);
            header->raw.parent = false;
            if (res)
                header->WriteBack(raw.nextHeader);

            delete header;
        }

        freeMap->WriteBack(freeMapFile);
    }

    return res;
}

/// Initialize a fresh file header for a newly created file.  Allocate data
/// blocks for the file out of the map of free disk blocks.  Return false if
/// there are not enough free blocks to accomodate the new file.
///
/// * `freeMap` is the bit map of free disk sectors.
/// * `fileSize` is the bit map of free disk sectors.
bool
FileHeader::Allocate(Bitmap *freeMap, unsigned fileSize)
{
    ASSERT(freeMap != nullptr);

    // if (fileSize > DISK_SIZE) {
    //     return false;
    // }

    raw.parent = true;
    raw.nextHeader = -1;
    raw.numBytes = fileSize < MAX_FILE_SIZE ? fileSize : MAX_FILE_SIZE;
    const unsigned int sectores = DivRoundUp(fileSize, SECTOR_SIZE);
    raw.numSectors = sectores < NUM_DIRECT ? sectores : NUM_DIRECT;
    for (unsigned i = 0; i < NUM_DIRECT; i++) 
                raw.dataSectors[i] = 0;

    if (!fileSize) {
        return true;
    }

    bool tmp = true;

    for (unsigned i = 0; i < raw.numSectors; i++) {
        raw.dataSectors[i] = freeMap->Find();
        DEBUG('f', "Found free sector\n");
    }

    if (sectores > raw.numSectors) {
        raw.nextHeader = freeMap->Find();
        if (raw.nextHeader == -1)
            return false;
        FileHeader* header = new FileHeader;
        // header->FetchFrom(raw.nextHeader);
        tmp = header->Allocate(freeMap, fileSize - (raw.numSectors*SECTOR_SIZE));
        DEBUG('f', "One indirection\n");
        header->raw.parent = false;
        if (tmp)
            header->WriteBack(raw.nextHeader);
        delete header;
    }

    return tmp;
}

/// De-allocate all the space allocated for data blocks for this file.
///
/// * `freeMap` is the bit map of free disk sectors.
void
FileHeader::Deallocate(Bitmap *freeMap)
{
    ASSERT(freeMap != nullptr);

    for (unsigned i = 0; i < NUM_DIRECT; i++)
        if (freeMap->Test(raw.dataSectors[i]) && raw.dataSectors[i])
            freeMap->Clear(raw.dataSectors[i]);

    if (raw.nextHeader != -1) {
        FileHeader* header = new FileHeader;
        header->FetchFrom(raw.nextHeader);
        header->Deallocate(freeMap);
        freeMap->Clear(raw.nextHeader);
        delete header;
    }
}

/// Fetch contents of file header from disk.
///
/// * `sector` is the disk sector containing the file header.
void
FileHeader::FetchFrom(unsigned sector)
{
    synchDisk->ReadSector(sector, (char *) &raw);
}

/// Write the modified contents of the file header back to disk.
///
/// * `sector` is the disk sector to contain the file header.
void
FileHeader::WriteBack(unsigned sector)
{
    synchDisk->WriteSector(sector, (char *) &raw);
}

/// Return which disk sector is storing a particular byte within the file.
/// This is essentially a translation from a virtual address (the offset in
/// the file) to a physical address (the sector where the data at the offset
/// is stored).
///
/// * `offset` is the location within the file of the byte in question.
unsigned
FileHeader::ByteToSector(unsigned offset)
{
    if (offset > (NUM_DIRECT * SECTOR_SIZE)) {
        ASSERT(raw.nextHeader);

        FileHeader* header = new FileHeader;
        header->FetchFrom(raw.nextHeader);
        unsigned tmp = header->ByteToSector(offset - (NUM_DIRECT * SECTOR_SIZE));
        delete header;
        return tmp;
    } else {
        return raw.dataSectors[offset / SECTOR_SIZE];
    }
}

/// Return the number of bytes in the file.
unsigned
FileHeader::FileLength() const
{
    int len = 0;
    if(raw.nextHeader != -1) {
        FileHeader* header = new FileHeader;
        header->FetchFrom(raw.nextHeader);
        len = header->FileLength();
        delete header;
    }

    return raw.numBytes + len;
}

/// Print the contents of the file header, and the contents of all the data
/// blocks pointed to by the file header.
void
FileHeader::Print(const char *title)
{
    if (raw.parent) {
        if (title == nullptr) {
            printf("File header:\n");
        } else {
            printf("%s file header:\n", title);
        }

        printf("    size: %u bytes\n"
            "    block indexes: ",
            raw.numBytes);        
    }

    PrintNumSector();
    printf("\n");
    PrintData();
}

void
FileHeader::PrintNumSector() {
    for (unsigned i = 0; i < raw.numSectors; i++) {
        printf("%u ", raw.dataSectors[i]);
    }

    if (raw.nextHeader != -1) {
        FileHeader* header = new FileHeader;
        header->FetchFrom(raw.nextHeader);
        header->PrintNumSector();
        delete header;
    }
}

void
FileHeader::PrintData() {
    char *data = new char [SECTOR_SIZE];

    for (unsigned i = 0, k = 0; i < raw.numSectors; i++) {
        printf("    contents of block %u:\n", raw.dataSectors[i]);
        synchDisk->ReadSector(raw.dataSectors[i], data);
        for (unsigned j = 0; j < SECTOR_SIZE && k < raw.numBytes; j++, k++) {
            if (isprint(data[j])) {
                printf("%c", data[j]);
            } else {
                printf("\\%X", (unsigned char) data[j]);
            }
        }
        printf("\n");
    }
    
    delete [] data;

    if (raw.nextHeader != -1) {
        FileHeader* header = new FileHeader;
        header->FetchFrom(raw.nextHeader);
        header->PrintNumSector();
        delete header;
    }
}

const RawFileHeader *
FileHeader::GetRaw() const
{
    return &raw;
}
