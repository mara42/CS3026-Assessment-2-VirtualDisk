#include "filesys.h"

// save contents of buffer to a block
// and give the buffer a new empty block
// also update FAT table
void saveBuffer(MyFILE*, fatentry_t);

// follow blockchain to next block
// to swap buffer with and reset pos
void newBuffer(MyFILE*);

// copies the content of the FAT into one or
// more blocks, then write these blocks to the virtual disk
void copyFAT();

// search FAT for the first unused FAT
// return -1 if none exist
fatentry_t findFree();

// Opens a file on the virtual disk and manages a buffer for it of size
// BLOCKSIZE, mode may be either “r” for readonly or “w” for read/write/append
// (default “w”)
MyFILE* myfopen(const char* filename, const char* mode);

// closes the file, writes out any blocks not written to disk
void myfclose(MyFILE* stream);

// Returns the next byte of the open file, or EOF (EOF == -1)
int myfgetc(MyFILE* stream);

// Writes a byte to the file. Depending on the write policy, either writes
// the disk block containing the written byte to disk, or waits until block
// is full
void myfputc(int b, MyFILE* stream);


// store a directory inside another, update parents next entry pointer
void saveDirEntry(diskblock_t* dirBlock, fatentry_t, fatentry_t loc, const char* name);

// create a directory block and return it's location
fatentry_t createDirBlock();

// this function will create a new directory, using path, e.g. mymkdir
// (“/first/second/third”) creates directory “third” in parent dir “second”,
// which is a subdir of directory “first”, and “first is a sub directory of
// the root directory
void mymkdir(const char* path);

// this function removes an existing directory, using path, e.g. myrmdir
// (“/first/second/third”) removes directory “third” in parent dir “second”,
// which is a subdir of directory “first”, and “first is a sub directory of
// the root directory
void myrmdir(const char* path);

// this function will change into an existing directory, using path, e.g.
// mkdir (“/first/second/third”) creates directory “third” in parent dir
// “second”, which is a subdir of directory “first”, and “first is a sub
// directory of the root directory
void mychdir(const char* path);

// this function removes an (“/first/second/third/testfile.txt”)
// existing file, using path, e.g. myremove
void myremove(const char* path);

// this function lists the content of a directory and returns a list of
// strings, where the last element is NULL
char** mylistdir(const char* path);
