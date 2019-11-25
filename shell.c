#include <stdio.h>
#include "filesys.h"
#include "helpers.h"
#include "string.h"
#include "stdlib.h"

int main(void){
  format();
  MyFILE* f = myfopen("testfile.txt", "w");
  // int fsize = 1;
  int fsize = BLOCKSIZE*4;
  for (int i = 0; i < fsize; i++) {
    myfputc(i % 26 + 65, f);
  }
  myfclose(f);
  writedisk("virtualdiskC3_C1");

  MyFILE* fr = myfopen("testfile.txt", "r");

  char testfile_contents[fsize];
  int c;
  for (int i=0; (c = myfgetc(fr)) != EOF; i++) {
    putchar(c);
    testfile_contents[i] = c;
  }
  testfile_contents[fsize] = 0;
  myfclose(fr);

  FILE* rf = fopen("testfileC3_C1_copy.txt", "w");
  if (f == 0) {
    printf("Error opening file!\n");
    exit(1);
  }
  fprintf(rf, "%s", testfile_contents);
  fclose(rf);

  return 0;
}