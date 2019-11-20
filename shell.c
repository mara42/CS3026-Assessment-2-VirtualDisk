#include <stdio.h>
#include "filesys.h"

int main(void){
  format();
  writedisk("virtualdisk_test");
  return 0;
}
