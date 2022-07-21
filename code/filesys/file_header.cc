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

    if (fileSize > MAX_FILE_SIZE) {
        return false;
    }

    raw.parent = true;
    raw.nextHeader = -1;
    raw.numBytes = fileSize;
    raw.numSectors = DivRoundUp(fileSize, SECTOR_SIZE);
    for (unsigned i = 0; i < NUM_DIRECT; i++) 
                raw.dataSectors[i] = 0;
    if (freeMap->CountClear() < raw.numSectors) {
        return false;  // Not enough space.
    }

    bool tmp = true;

    for (unsigned i = 0; i <= NUM_DIRECT && i < raw.numSectors; i++) {
        if (i != NUM_DIRECT) {
            raw.dataSectors[i] = freeMap->Find();
        } else {
            raw.nextHeader = freeMap->Find();
            if (raw.nextHeader == -1)
                return false;

            FileHeader* header = FetchFrom(raw.nextHeader);
            tmp = header->Allocate(freeMap, raw.numBytes - (NUM_DIRECT * SECTOR_SIZE));
            header->raw.parent = false;
            if (tmp)
                header->WriteBack();

            delete header;
        }
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
        FileHeader* header = FetchFrom(raw.nextHeader);
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

        FileHeader* header = FetchFrom(raw.nextHeader);
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
    ASSERT(raw.parent);
    return raw.numBytes;
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
    for (unsigned i = 0; i < NUM_DIRECT && i < raw.numSectors; i++) {
        printf("%u ", raw.dataSectors[i]);
    }

    if (raw.nextHeader != -1) {
        FileHeader* header = FetchFrom(raw.nextHeader);
        header->PrintNumSector();
        delete header;
    }
}

void
FileHeader::PrintData() {
    char *data = new char [SECTOR_SIZE];

    for (unsigned i = 0, k = 0; i < NUM_DIRECT && i < raw.numSectors; i++) {
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
        FileHeader* header = FetchFrom(raw.nextHeader);
        header->PrintNumSector();
        delete header;
    }
}

const RawFileHeader *
FileHeader::GetRaw() const
{
    return &raw;
}
