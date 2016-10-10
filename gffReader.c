

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "gffTypes.h"
#include "gffWriter.h"

#define FALSE 0
#define TRUE !FALSE


#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

void DumpStruct(gff_struct *gs, int level);

void DumpList(gff_list *list, int level);


int readHeader(FILE *f, GFF_HEADER *header) {

  memset(header, '\0', sizeof(GFF_HEADER));

  fseek(f, 0, SEEK_SET);

//  printf("FileType=%s\n",header->FileType);
  fread(header->FileType, 4, 1, f);
  fread(header->FileVersion, 4, 1, f);

  if(strcmp(header->FileVersion, "V3.2") != 0) {
    printf("Invalid version %s",header->FileVersion);
    return -1;
  }

  fread(&(header->StructOffset), sizeof(dword), 1, f);
  fread(&(header->StructCount), sizeof(dword), 1, f);
  fread(&(header->FieldOffset), sizeof(dword), 1, f);
  fread(&(header->FieldCount), sizeof(dword), 1, f);
  fread(&(header->LabelOffset), sizeof(dword), 1, f);
  fread(&(header->LabelCount), sizeof(dword), 1, f);
  fread(&(header->FieldDataOffset), sizeof(dword), 1, f);
  fread(&(header->FieldDataCount), sizeof(dword), 1, f);
  fread(&(header->FieldIndiciesOffset), sizeof(dword), 1, f);
  fread(&(header->FieldIndiciesCount), sizeof(dword), 1, f);
  fread(&(header->ListIndiciesOffset), sizeof(dword), 1, f);
  fread(&(header->ListIndiciesCount), sizeof(dword), 1, f);

  PrintHeaderCheck(header);

  printf("pos=%d\n", ftell(f));
}

void PrintHeaderCheck(GFF_HEADER *header) {
//  fread(header->, sizeof(dword), 1, f);

  printf("FileType=%s, FileVersion=%s \nSO=%d, SC=%d\n",header->FileType,
         header->FileVersion, header->StructOffset, header->StructCount);

  printf("Fo=%d; Fc=%d\n", header->FieldOffset, header->FieldCount);
  printf("Lo=%d; Lc=%d\n", header->LabelOffset, header->LabelCount);
  printf("FDo=%d; FDc=%d\n", header->FieldDataOffset, header->FieldDataCount);
  printf("FIo=%d; FIc=%d\n", header->FieldIndiciesOffset, header->FieldIndiciesCount);
  printf("LIo=%d; LIc=%d\n", header->ListIndiciesOffset, header->ListIndiciesCount);
}

void ReadStructs(FILE *f, gff_struct *gfs, int cnt, dword offset) {
  int i;

  memset(gfs, '\0', sizeof(gff_struct) * cnt);
  fseek(f, offset, SEEK_SET);

  for(i = 0; i < cnt; i++) {
    fread(&(gfs[i].Type), sizeof(dword), 1, f);
    fread(&(gfs[i].DataOrDataOffset), sizeof(dword), 1, f);
    fread(&(gfs[i].FieldCount), sizeof(dword), 1, f);
    // short dump
//    if(i <= 20)
//      printf("#%d Type=%#08x offset=%d; Fc=%d\n", i,  gfs[i].Type,  gfs[i].DataOrDataOffset,  gfs[i].FieldCount);
  }
  printf("pos=%d\n", ftell(f));

}

void ReadFields(FILE *f, gff_field *gf_f, int cnt, dword offset) {
  int i;

  memset(gf_f, '\0', sizeof(gff_field) * cnt);
  fseek(f, offset, SEEK_SET);

  for(i = 0; i < cnt; i++) {
    fread(&(gf_f[i].Type), sizeof(dword), 1, f);
    fread(&(gf_f[i].LabelIndex), sizeof(dword), 1, f);
    fread(&(gf_f[i].DataOrDataOffset), sizeof(dword), 1, f);
    // short dump
//    if(i <= 20)
//      printf("#%d Type=%d LabelIndex=%d; DataOrDataOffset=%d\n", i,  gf_f[i].Type, gf_f[i].LabelIndex,  gf_f[i].DataOrDataOffset);
  }
  printf("pos=%d\n", ftell(f));

}

int IsFieldComplex(GFF_FIELD_TYPE type) {
  if(type >= 6 && type != GFF_FLOAT)
    return TRUE;

  return FALSE;
}

void ReadLabels(FILE *f, gff_label *gf_l, int cnt, dword offset) {
  int i;

  memset(gf_l, '\0', sizeof(gff_label) * cnt);

  fseek(f, offset, SEEK_SET);

  for(i = 0; i < cnt; i++) {
    fread(gf_l[i], sizeof(char), 16, f);
//    if(i <= 20)
//      printf("#%d Label=%s\n", i,  gf_l[i]);
  }

  printf("pos=%d\n", ftell(f));

}

void *ReadFieldData(FILE *f, int cnt, dword offset) {

  char *data = malloc(sizeof(char) * cnt);
  memset(data, '\0', sizeof(char) * cnt);

  fseek(f, offset, SEEK_SET);
  fread(data, sizeof(char), cnt, f);

  printf("pos=%d\n", ftell(f));
  return data;

}

dword *ReadFieldIndices(FILE *f, int cnt, dword offset) {

  dword *data = malloc(sizeof(dword) * cnt);
  memset(data, '\0', sizeof(dword) * cnt);

  fseek(f, offset, SEEK_SET);
  fread(data, sizeof(char), cnt, f);

  printf("pos=%d\n", ftell(f));
  return data;
}

gff_list *GetListByOffset(gff_list *lists, dword offset, int size) {
  int i;

  for(i = 0; i < size; i++) {
    if(lists[i].__offset == offset)
      return &(lists[i]);
    if(lists[i].last)
      break;
  }

  printf("%d No offset %d\n",size, offset);
  return NULL;
}

gff_list *ReadListIndicies(FILE *f, int cnt, dword offset) {

  /* Size guess:
   * We have 'cnt' bytes of data. Since every item consist of 4bytes,
   * if each array would have 0 items, it would be cnt/4 empty lists.
   */
  gff_list *ListIndicies = malloc(sizeof(gff_list) * cnt/4);
  memset(ListIndicies, '\0', sizeof(gff_list) * cnt/4);

  int s=0;
  int i,j;

  fseek(f, offset, SEEK_SET);

  for(i = 0; s < cnt; i++) {
    fread(&(ListIndicies[i].Size), sizeof(dword), 1, f);
//    if(i<=20)
//      printf("\n#%d [%d]\n    ", i, ListIndicies[i].Size);
    ListIndicies[i].__offset = s;
    s += sizeof(dword);

    ListIndicies[i].Data = malloc(sizeof(dword) * ListIndicies[i].Size);
    memset(ListIndicies[i].Data, '\0', sizeof(dword) * ListIndicies[i].Size);

    fread(ListIndicies[i].Data, sizeof(dword), ListIndicies[i].Size, f);
    s += sizeof(dword) * ListIndicies[i].Size;

//    printf("\n#%d [%d] on %d\n    ", i, ListIndicies[i].Size, ListIndicies[i].__offset);
/*    if(i<=20)
      for(j = 0; j < ListIndicies[i].Size; j++) {
        printf("%d ", ListIndicies[i].Data[j]);
      }*/

    ListIndicies[i].last = FALSE;
    if(s >= cnt)
      ListIndicies[i].last = TRUE;
  }
//  printf("\npos=%d\n", ftell(f));
  return ListIndicies;

}

char *dumpFieldValue(gff_field *gf) {
  static char dump[17];
  memset(dump, '\0', 17);

  switch(gf->Type) {
    case GFF_BYTE:
//      snprintf(dump, 17, "%#08x", gf->v_byte);
      snprintf(dump, 17,ANSI_COLOR_GREEN "%d"ANSI_COLOR_RESET, gf->v_byte);
      break;
    case GFF_CHAR:
      snprintf(dump, 17,ANSI_COLOR_GREEN "%d"ANSI_COLOR_RESET, gf->v_char);
      break;
    case GFF_WORD:
      snprintf(dump, 17, "%d", gf->v_word);
      break;
    case GFF_SHORT:
      snprintf(dump, 17, "%d", gf->v_short);
      break;
    case GFF_DWORD:
      snprintf(dump, 17, "%d", gf->v_dword);
      break;
    case GFF_INT:
      snprintf(dump, 17, "%d", gf->v_int);
      break;
    case GFF_DWORD64:
      snprintf(dump, 17, "%ld", gf->v_dword64);
      break;
    case GFF_FLOAT:
      snprintf(dump, 17, "%f", gf->v_float);
      break;
    case GFF_DOUBLE:
      snprintf(dump, 17, "%lf", gf->v_double);
      break;
    case GFF_CEXOSTRING:
      return gf->v_cexostring;
    case GFF_RESREF:
      snprintf(dump, 17, "%s", gf->v_resref);
      break;
/*    case GFF_CEXOLOCSTRING:
    case GFF_VOID:
    case GFF_STRUCT:
    case GFF_LIST:
    case GFF_RESERVED:
    case GFF_VECTOR: */
  }
  return dump;
}

void DumpField(gff_field *gf, int level) {

  printf("%*s t(%d) label(%d)%s val:%s \n", level, "",gf->Type, gf->LabelIndex, gf->Label, dumpFieldValue(gf));

  if(gf->Type == GFF_STRUCT) {
    DumpStruct(gf->v_struct, level+2);
  }
  if(gf->Type == GFF_LIST) {
    if(gf->v_list != NULL)
      DumpList(gf->v_list, level+2);
  }
}

void DumpStruct(gff_struct *gs, int level) {
  int i;
  printf("%*s %#08x Fc=%d\n", level, "", gs->Type, gs->FieldCount);
  for(i=0 ; i< gs->FieldCount; i++) {
    DumpField(gs->fields[i], level+2);
  }
}

void DumpList(gff_list *list, int level) {
  int i;
  for(i = 0; i < list->Size; i++) {
    DumpStruct(list->structs[i], level);
  }
}

void ProcessStruct(GFF_HEADER *header, gff_struct *gs) {
  int i;

  gs->fields = malloc(sizeof(void*) * gs->FieldCount);
  memset(gs->fields, '\0', sizeof(void*) * gs->FieldCount);

  if(gs->FieldCount == 1) {
    //  this is an index into the Field Array.
    gs->fields[0] = &(header->Fields[gs->DataOrDataOffset]);
  }
  else {
    //  this is a byte offset into the Field Indices array.
    dword pos = gs->DataOrDataOffset / sizeof(dword);
    for(i = 0; i < gs->FieldCount; i++ ) {
      dword ref = header->FieldIndices[pos + i];
      gs->fields[i] = &(header->Fields[ref]);
    }
  }

  //DumpStruct(gs, 1);

}

void ProcessLists(GFF_HEADER *header) {
  int i,j;
  gff_list *l;

  for(i = 0; i < header->ListIndiciesCount/4; i++) {
    l = &(header->ListIndicies[i]);

    l->structs = malloc(sizeof(gff_struct_p) * l->Size);
    for(j = 0; j < l->Size; j++) {
      l->structs[j] = &(header->Structs[l->Data[j]]);
    }
    if(l->last)
      break;
  }

}

void FieldToValue(gff_field *gf, GFF_HEADER *header) {
  char *ref;
  dword size;

  switch(gf->Type) {
    case GFF_BYTE:
    case GFF_CHAR:
      memcpy(&(gf->v_char), &(gf->DataOrDataOffset), 1);
      break;

    case GFF_SHORT:
    case GFF_WORD:
      memcpy(&(gf->v_short), &(gf->DataOrDataOffset), 2);
      break;

    case GFF_DWORD:
      memcpy(&(gf->v_dword), &(gf->DataOrDataOffset), 4);
      break;
    case GFF_FLOAT:
      memcpy(&(gf->v_float), &(gf->DataOrDataOffset), 4);
      break;
    case GFF_INT:
      memcpy(&(gf->v_int), &(gf->DataOrDataOffset), 4);
      break;

    case GFF_DWORD64:
      ref = header->FieldDataBlock + gf->DataOrDataOffset;
      memcpy(&(gf->v_dword64), ref, sizeof(gff_dword64));
      break;
    case GFF_INT64:
      ref = header->FieldDataBlock + gf->DataOrDataOffset;
      memcpy(&(gf->v_int64), ref, sizeof(gff_int64));
      break;
    case GFF_DOUBLE:
      ref = header->FieldDataBlock + gf->DataOrDataOffset;
      memcpy(&(gf->v_double), ref, sizeof(gff_double));
      break;
    case GFF_CEXOSTRING:
      ref = header->FieldDataBlock + gf->DataOrDataOffset;
      memcpy(&size, ref, sizeof(gff_dword));
      gf->v_cexostring = malloc(sizeof(char) * size + 1);
      memset(gf->v_cexostring,'\0',size + 1);
      memcpy(gf->v_cexostring, ref+ sizeof(gff_dword), size);
      break;
    case GFF_RESREF:
      ref = header->FieldDataBlock + gf->DataOrDataOffset;
      memcpy(&size, ref, sizeof(char));
      gf->v_resref = malloc(sizeof(char) * size + 1);
      memset(gf->v_resref,'\0',size + 1);
      memcpy(gf->v_resref, ref+ sizeof(char), size);
      break;
//    case GFF_CEXOLOCSTRING: Do not need to work with these strings
//    case GFF_VOID: //Not interesetd
    case GFF_STRUCT:
      gf->v_struct = &(header->Structs[gf->DataOrDataOffset]);
      break;
    case GFF_LIST:
      gf->v_list = GetListByOffset(header->ListIndicies, gf->DataOrDataOffset, header->ListIndiciesCount/4);
      break;
//    case GFF_RESERVED:
//    case GFF_VECTOR:
  }

}

void ProcessField(GFF_HEADER *header, gff_field *gf) {

  // Set label pointer
  gf->Label = header->Labels[gf->LabelIndex];

  //convert raw data into value
  FieldToValue(gf, header);

}

GFF_HEADER *ReadGffFile(const char *filename) {

  int i;


  GFF_HEADER *header;
  header = malloc(sizeof(GFF_HEADER));

  FILE *f = fopen(filename, "rb");

  readHeader(f, header);

  header->Structs = malloc(sizeof(gff_struct) * header->StructCount);
  ReadStructs(f, header->Structs, header->StructCount, header->StructOffset);

  header->Fields = malloc(sizeof(gff_field) * header->FieldCount);
  ReadFields(f, header->Fields, header->FieldCount, header->FieldOffset);

  header->Labels = malloc(sizeof(gff_label) * header->LabelCount);
  ReadLabels(f, header->Labels, header->LabelCount, header->LabelOffset);

  header->FieldDataBlock = ReadFieldData(f, header->FieldDataCount, header->FieldDataOffset);

  header->FieldIndices = ReadFieldIndices(f, header->FieldIndiciesCount, header->FieldIndiciesOffset);

  header->ListIndicies = ReadListIndicies(f, header->ListIndiciesCount, header->ListIndiciesOffset);
  fclose(f);

  //All data read from file

  ProcessLists(header);
  for(i = 0; i < header->StructCount; i++)
    ProcessStruct(header, &(header->Structs[i]));
  for(i = 0; i < header->FieldCount; i++)
    ProcessField(header, &(header->Fields[i]));

//  DumpStruct(&(header->Structs[0]),1);

  return header;

}
