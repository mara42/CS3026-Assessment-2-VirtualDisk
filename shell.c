#include <stdio.h>
#include "filesys.h"
#include "helpers.h"
#include "string.h"
#include "stdlib.h"

int main(void){
  format();
  MyFILE* f = myfopen("testfile.txt", "w");
  int fsize = BLOCKSIZE*4;
  for (int i = 0; i < fsize; i++) {
    myfputc(i % 26 + 65, f);
  }
  myfclose(f);
  writedisk("virtualdiskC3_C1");

  MyFILE* fr = myfopen("testfile.txt", "r");
  if (fr == 0) {
    printf("Error opening file!\n");
    exit(1);
  }

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

  mymkdir("/myfirstdir/myseconddir/mythirddir");
  char** ls = mylistdir("/myfirstdir/myseconddir");
  for (int i=0; ls[i]; i++) {
    puts(ls[i]);
    free(ls[i]);
  }
  free(ls);
  writedisk("virtualdiskB3_B1_a");

  // MyFILE* ft = myfopen("/myfirstdir/myseconddir/testfile.txt", "w");
  // myfclose(ft);
  // char** ls2 = mylistdir("/myfirstdir/myseconddir");
  // for (int i=0; ls2[i]; i++) {
  //   puts(ls2[i]);
  //   free(ls2[i]);
  // }
  // free(ls2);
  // writedisk("virtualdiskB3_B1_b");

  return 0;
}