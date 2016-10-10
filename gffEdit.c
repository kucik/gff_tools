#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "gffTypes.h"
#include "gffWriter.h"


void main( const int argc, const char **argv ) {

  int i;

  printf("sizeof ushort= %d uint=%d dword=%d\n", sizeof(unsigned short), sizeof(unsigned int), sizeof(dword));
  printf("sizeof float=%d, double=%d, long=%d, ulong=%d\n", sizeof(float), sizeof(double), sizeof(long), sizeof(unsigned long));

  if(argc <= 1) {
    return;
  }


  GFF_HEADER *header = ReadGffFile(argv[1]);

//  DumpStruct(&(header->Structs[0]),1);

  PrepareDataWrite(header);
  PrintHeaderCheck(header);

  char filename[100];
  sprintf(filename, "%s.new", argv[1]);
  WriteDownFile(filename, header);


}
