/* filesys.c
 *
 * provides interface to virtual disk
 *
 */
#include "filesys.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "helpers.h"

diskblock_t virtualDisk[MAXBLOCKS];  // define our in-memory virtual, with
                                     // MAXBLOCKS blocks
fatentry_t FAT[MAXBLOCKS];    // define a file allocation table with MAXBLOCKS
                              // 16-bit entries
fatentry_t rootDirIndex = 0;  // rootDir will be set by format
direntry_t* currentDir = NULL;
fatentry_t currentDirIndex = 0;

/* writedisk : writes virtual disk out to physical disk
 *
 * in: file name of stored virtual disk */
void writedisk(const char* filename) {
  printf("writedisk> virtualdisk[0] = %s\n", virtualDisk[0].data);
  FILE* dest = fopen(filename, "w");
  if (fwrite(virtualDisk, sizeof(virtualDisk), 1, dest) < 0)
    fprintf(stderr, "write virtual disk to disk failed\n");
  // write(dest, virtualDisk, sizeof(virtualDisk));
  fclose(dest);
}

void readdisk(const char* filename) {
  FILE* dest = fopen(filename, "r");
  if (fread(virtualDisk, sizeof(virtualDisk), 1, dest) < 0)
    fprintf(stderr, "write virtual disk to disk failed\n");
  // write( dest, virtualDisk, sizeof(virtualDisk) ) ;
  fclose(dest);
}

/* the basic interface to the virtual disk
 * this moves memory around */
void writeblock(diskblock_t* block, int block_address) {
  // printf ( "writeblock> block %d = %s\n", block_address, block->data ) ;
  memmove(virtualDisk[block_address].data, block->data, BLOCKSIZE);
  // printf ( "writeblock> virtualdisk[%d] = %s / %d\n", block_address,
  // virtualDisk[block_address].data, (int)virtualDisk[block_address].data ) ;
}

/* read and write FAT
 *
 * please note: a FAT entry is a short this is a 16-bit word, or 2 bytes
 *              our blocksize for the virtual disk is 1024, therefore
 *              we can store 512 FAT entries in one block
 *
 *              how many disk blocks do we need to store the complete FAT:
 *              - our virtual disk has MAXBLOCKS blocks, which is currently 1024
 *                each block is 1024 bytes long
 *              - our FAT has MAXBLOCKS entries, which is currently 1024
 *                each FAT entry is a fatentry_t, which is currently 2 bytes
 *              - we need (MAXBLOCKS /(BLOCKSIZE / sizeof(fatentry_t))) blocks
 *                to store the FAT
 *              - each block can hold (BLOCKSIZE / sizeof(fatentry_t)) fat
 *                entries
 */
void readblock(diskblock_t* block, int block_address) {
  memmove(block->data, virtualDisk[block_address].data, BLOCKSIZE);
}

/* prepare block 0 : fill it with '\0',
 * use strcpy() to copy some text to it for test purposes
 * write block 0 to virtual disk
 * prepare FAT table
 * write FAT blocks to virtual disk
 * prepare root directory
 * write root directory block to virtual disk
 */
void format() {
  int pos = 0;
  int fatentry = 0;

  diskblock_t block = {0};
  memmove(block.data, (Byte*)"S3008 Operating Systems Assessment 2012", 40);
  writeblock(&block, 0);

  // memset(FAT + sizeof(fatentry_t), UNUSED, sizeof(FAT)-sizeof(fatentry_t));
  memset(FAT, UNUSED, sizeof(FAT));
  FAT[0] = 0;

  // unfortunately this might be making something that's not supposed to be
  // generic, generic.
  for (int i = 1; i < FATBLOCKSNEEDED; i++) {
    FAT[i] = i + 1;
  }
  FAT[FATBLOCKSNEEDED] = ENDOFCHAIN;
  rootDirIndex = FATBLOCKSNEEDED + 1;
  FAT[rootDirIndex] = ENDOFCHAIN;  // this is for the dirblock below and placed
                                   // here to reduce writes
  copyFAT();

  diskblock_t rootDir = {0};
  rootDir.dir = (dirblock_t){.isdir = true, .nextEntry = 0, .entrylist = 0};

  writeblock(&rootDir, rootDirIndex);

  // maybe not right to define it here?
  currentDirIndex = rootDirIndex;
}

void copyFAT() {
  int increment =
      MAXBLOCKS / FATBLOCKSNEEDED * sizeof(fatentry_t);  // make it look nicer?
  for (int i = 0; i < FATBLOCKSNEEDED; i++) {
    diskblock_t tmp;
    memmove(tmp.fat, FAT + increment * i, increment);
    writeblock(&tmp, i + 1);
  }
}

/* use this for testing */

void printBlock(int blockIndex) {
  printf("virtualdisk[%d] = %s\n", blockIndex, virtualDisk[blockIndex].data);
}

fatentry_t searchDir(dirblock_t* dir, const char* name) {
  for (int i = 0; i < dir->nextEntry; i++) {
    direntry_t entry = dir->entrylist[i];
    if (strcmp(entry.name, name) == 0) {
      return entry.firstblock;
    }
  }
  return 0;
}

MyFILE* myfopen(const char* filename, const char* mode) {
  MyFILE* f = malloc(sizeof(MyFILE));
  *f = (MyFILE){0};
  strcpy(f->mode, mode);

  int dirIndex = currentDirIndex;
  diskblock_t* dirBlock = calloc(sizeof(diskblock_t), 0);
  readblock(dirBlock, dirIndex);
  dirblock_t* dir = &dirBlock->dir;

  /*  open existing file */
  f->blockno = searchDir(dir, filename);

  if (!f->blockno && strcmp(mode, "r") == 0)
    return 0;

  readblock(&f->buffer, f->blockno);


  // for (int i = 0; i < dir->nextEntry; i++) {
  //   direntry_t entry = dir->entrylist[i];
  //   if (strcmp(entry.name, filename) == 0 && !entry.isdir) {
  //     f->blockno = entry.firstblock;
  //     readblock(&f->buffer, entry.firstblock);
  //   }
  // }

  /* new file */
  if (!f->blockno) {
    f->blockno = findFree();
    direntry_t entry = (direntry_t){
        .entrylength = 0,
        .isdir = false,
        .unused = false,
        .modtime = 0,
        .filelength = 0,
        .firstblock = f->blockno,
    };
    strcpy(entry.name, filename);
    dirBlock->dir.entrylist[dir->nextEntry++] = entry;

    FAT[f->blockno] = ENDOFCHAIN;
    copyFAT();
    writeblock(dirBlock, dirIndex);
    free(dirBlock);
  }

  // tell direntry_t where it's located
  f->dirBlockNo = dirIndex;
  f->dirEntryNo = dir->nextEntry;

  return f;
}

// TODO: do we need to support multiple open files at once?
fatentry_t findFree() {
  for (int i = rootDirIndex + 1; i < MAXBLOCKS; i++) {
    if (FAT[i] == UNUSED)
      return i;
  }
  return -1;
}

void myfclose(MyFILE* stream) {
  diskblock_t dir = {0};
  readblock(&dir, stream->dirBlockNo);
  direntry_t* entry = &dir.dir.entrylist[stream->dirEntryNo];
  int chain = entry->firstblock;
  while ((chain = FAT[chain]) != ENDOFCHAIN) {
    entry->entrylength++;
  }
  entry->filelength = BLOCKSIZE * entry->entrylength + stream->pos;
  writeblock(&dir, stream->dirBlockNo);
  saveBuffer(stream, ENDOFCHAIN);
  free(stream);
}

int myfgetc(MyFILE* stream) {
  if (FAT[stream->blockno] < 1) {
    diskblock_t dir = {0};
    readblock(&dir, stream->dirBlockNo);  // this check should most def be done
                                          // another way e.g. storing remaining
                                          // counter in the stream itself
    int remaining =
        (dir.dir.entrylist[stream->dirEntryNo].filelength - 1) % (BLOCKSIZE) +
        1;
    if (remaining == stream->pos) {
      return EOF;
    }
  }
  // int index = stream->pos++;
  if (stream->pos == BLOCKSIZE) {
    newBuffer(stream);
  }

  return stream->buffer.data[stream->pos++];
}

void newBuffer(MyFILE* stream) {
  stream->blockno = FAT[stream->blockno];
  readblock(&stream->buffer, stream->blockno);
  stream->pos = 0;
}

void myfputc(int b, MyFILE* stream) {
  if (stream->pos == BLOCKSIZE) {
    saveBuffer(stream, findFree());
  }
  stream->buffer.data[stream->pos++] = b;
}

void saveBuffer(MyFILE* stream, fatentry_t next) {
  writeblock(&stream->buffer, stream->blockno);

  FAT[stream->blockno] = next;
  FAT[next] = ENDOFCHAIN;
  copyFAT();
  stream->blockno = next;

  memset(stream->buffer.data, 0, BLOCKSIZE);
  stream->pos = 0;
}

void saveDirEntry(diskblock_t* dirBlock, fatentry_t parentLoc, fatentry_t childLoc, const char* name){
  direntry_t entry = (direntry_t){
      .entrylength = 0,
      .isdir = true,
      .unused = false,
      .modtime = 0,
      .filelength = 0,
      .firstblock = childLoc
  };
  strcpy(entry.name, name);
  dirBlock->dir.entrylist[dirBlock->dir.nextEntry++] = entry;
  writeblock(dirBlock, parentLoc);
}

fatentry_t createDirBlock(void) {
  fatentry_t loc = findFree();

  FAT[loc] = ENDOFCHAIN;
  copyFAT();

  diskblock_t newDir = {0};
  newDir.dir = (dirblock_t){.isdir = true, .nextEntry = 0, .entrylist = 0};
  writeblock(&newDir, loc);

  return loc;
}

void mymkdir(const char* path) {
  diskblock_t dirBlock;
  fatentry_t dirLoc;

  if (path[0] == '/') {
    readblock(&dirBlock, rootDirIndex);
    path++;
    dirLoc = rootDirIndex;
  } else {
    readblock(&dirBlock, currentDirIndex);
    dirLoc = currentDirIndex;
  }

  char* rest = NULL;
  char* token;
  char* copy = strdup(path);
  for (token = strtok_r(copy, "/", &rest);
       token != NULL;
       token = strtok_r(NULL, "/", &rest)) {
    fatentry_t loc = searchDir(&dirBlock.dir, token);
    if (loc == 0) {
      loc = createDirBlock();
      saveDirEntry(&dirBlock, dirLoc, loc, token);
    }
    readblock(&dirBlock, loc);
    dirLoc = loc;
  }
  free(copy);
}

char** mylistdir(const char* path) {
  diskblock_t dirBlock;
  fatentry_t dirLoc;
  char* copy = strdup(path);
  if (copy[0] == '/') {
    readblock(&dirBlock, rootDirIndex);
    copy++;
    dirLoc = rootDirIndex;
  } else {
    readblock(&dirBlock, currentDirIndex);
    dirLoc = currentDirIndex;
  }

  char* rest = NULL;
  char* token;
  if (strcmp(path, "/") == 0) {
    readblock(&dirBlock, rootDirIndex);
  } else{
  for (token = strtok_r(copy, "/", &rest);
       token != NULL;
       token = strtok_r(NULL, "/", &rest)) {
    fatentry_t loc = searchDir(&dirBlock.dir, token);
    if (loc == 0) {
      return 0;
    }
    readblock(&dirBlock, loc);
    dirLoc = loc;
  }
  }
  char** contents = malloc(sizeof(char*)*dirBlock.dir.nextEntry);
  for (int i = 0; i < dirBlock.dir.nextEntry; i++) {
    contents[i] = strdup(dirBlock.dir.entrylist[i].name);
  }
  contents[dirBlock.dir.nextEntry] = 0;

  return contents;
}

/* B  ^^^^^^
- mymkdir( char * path) that creates a new directory
- char ** mylistdir (char * path) that lists the content of a directory
*/

void myrmdir(const char* path) {}

void mychdir(const char* path) {}

void myremove(const char* path) {}
