/* filesys.h
 *
 * describes FAT structures
 * http://www.c-jump.com/CIS24/Slides/FAT/lecture.html#F01_0020_fat
 * http://www.tavi.co.uk/phobos/fat.html
 */

#ifndef FILESYS_H
#define FILESYS_H

#include <time.h>

/*
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
*/

#define MAXBLOCKS 1024
#define BLOCKSIZE 1024
#define FATENTRYCOUNT (BLOCKSIZE / sizeof(fatentry_t))  // expect to be 512
#define DIRENTRYCOUNT \
  ((BLOCKSIZE - (2 * sizeof(int))) / sizeof(direntry_t))  // ~3
#define MAXNAME 256
#define MAXPATHLENGTH 1024

#define FATBLOCKSNEEDED (MAXBLOCKS / FATENTRYCOUNT)  // expect to be 2

#define UNUSED -1
#define ENDOFCHAIN 0

#ifndef EOF
#define EOF -1
#endif

typedef unsigned char Byte;

/* create a type fatentry_t, we set this currently to short (16-bit)
   NOTE: I replaced short with int16_t to be more explicit and to ensure
   16-bit length at all times */
typedef int16_t fatentry_t;

// const int   fatentrycount = (blocksize / sizeof(fatentry_t)) ;

// a FAT block is a list of 16-bit entries that form a chain of disk addresses
typedef fatentry_t fatblock_t[FATENTRYCOUNT];

/*
int entrylength          : block count?
Byte isdir               : is dir or not
Byte unused              : ????
time_t modtime           : ????
int filelength            : byte count?
fatentry_t firstblock     : which is the first diskblock_t
char name[MAXNAME]       : filename

one of the lengths is supposed to be byte count
created on fclose() call
*/
typedef struct direntry {
  int entrylength;  // records length of this entry (can be used with names of
                    // variables length)
  Byte isdir;
  Byte unused;
  time_t modtime;
  int filelength;
  fatentry_t firstblock;
  char name[MAXNAME];
} direntry_t;


// const int   direntrycount = (blocksize - (2*sizeof(int)) ) /
// sizeof(direntry_t) ;

// a directory block is an array of directory entries
// int isdir                            : boolean
// int nextEntry                        : next free entrylist index
// direntry_t entrylist[DIRENTRYCOUNT]  : files inside directory
typedef struct dirblock {
  int isdir;
  int nextEntry;
  direntry_t
      entrylist[DIRENTRYCOUNT];  // the first two integer are marker and endpos
} dirblock_t;

// a data block holds the actual data of a filelength, it is an array of 8-bit
// (byte) elements
typedef Byte datablock_t[BLOCKSIZE];

// a diskblock can be either a directory block, a FAT block or actual data
// datablock_t data   : actual file data to be read
// dirblock_t dir     : directory metadata
// fatblock_t fat     : used for VD[1]&[2] for organizing data
typedef union block {
  datablock_t data;
  dirblock_t dir;
  fatblock_t fat;
} diskblock_t;

// finally, this is the disk: a list of diskblocks
// the disk is declared as extern, as it is shared in the program
// it has to be defined in the main program filelength
extern diskblock_t virtualDisk[MAXBLOCKS];

/*
when a file is opened on this disk, a file handle has to be
created in the opening program

char mode[3]           : either "r" or "w"
fatentry_t blockno     : location in FAT
int pos                : current buffer position
diskblock_t buffer     : datablock_t used for writing or reading
*/
typedef struct filedescriptor {
  char mode[3];
  fatentry_t blockno;  // block no
  fatentry_t dirBlockNo;
  int dirEntryNo;
  int pos;             // byte within a block
  diskblock_t buffer;
} MyFILE;

// creates the initial structure on the virtual disk, writing the FAT and
// the root directory into the virtual disk
void format();
void writedisk(const char* filename);

#endif

/*
#define NUM_TYPES (sizeof types / sizeof types[0])
static* int types[] = {
    1,
    2,
    3,
    4 };
*/