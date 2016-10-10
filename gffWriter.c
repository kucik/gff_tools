
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "gffTypes.h"


void *MakeStructsBlob(int *size, gff_struct *gfs, int count) {
  int i;
  dword *p;

  /* Calculate needed memory */
  void *data = malloc(sizeof(dword) * 3 * count);
  memset(data, '\0',sizeof(dword) * 3 * count);
  p = data;

  for(i = 0; i < count; i++) {
    memcpy(p, &(gfs[i].Type), sizeof(dword));
    p++;
    memcpy(p, &(gfs[i].DataOrDataOffset), sizeof(dword));
    p++;
    memcpy(p, &(gfs[i].FieldCount), sizeof(dword));
    p++;
  }

  printf("Structs: Written %d structs into %d(%d) bytes\n", count, (void *)p - data,
         sizeof(dword) * 3 * count);
  *size = (void *)p - data;
  return data;
}

void *MakeFieldsBlob(int *size, gff_field *gfs, int count) {
  int i;
  dword *p;

  /* Calculate needed memory */
  void *data = malloc(sizeof(dword) * 3 * count);
  memset(data, '\0',sizeof(dword) * 3 * count);
  p = data;

  for(i = 0; i < count; i++) {
    memcpy(p, &(gfs[i].Type), sizeof(dword));
    p++;
    memcpy(p, &(gfs[i].LabelIndex), sizeof(dword));
    p++;
    memcpy(p, &(gfs[i].DataOrDataOffset), sizeof(dword));
    p++;
  }

  printf("Fields: Written %d fields into %d(%d) bytes\n", count, (void *)p - data,
         sizeof(dword) * 3 * count);
  *size = (void *)p - data;
  return data;
}

void *MakeLabelsBlob(int *size, gff_label *gfl, int count) {
  int i;
  char *p;

  /* Calculate needed memory */
  void *data = malloc(sizeof(char) * 16 * count);
  memset(data, '\0',sizeof(char) * 16 * count);
  p = data;

  for(i = 0; i < count; i++) {
    memcpy(p, &(gfl[i]), sizeof(char) * 16);
    p +=16;
  }
  printf("Labels: Written %d labels into %d(%d) bytes\n", count, (void *)p - data,
         sizeof(char) * 16 * count);
  *size = (void *)p - data;
  return data;
}

/**
 * FieldData and FieldIndices does not need to be reformated for dump
 */

void *MakeListIndicesBlob(int *size, gff_list *gfl, int count) {

  int s = 0;
  int i;
  void *data = NULL;
  dword *p = NULL;

  /* Calculate needed memory */
  for(i = 0; i < count/4; i++) {
    /* Add space for list header */
    s += sizeof(dword);
    s += sizeof(dword) * gfl[i].Size;

    if(gfl[i].last) {
      break;
    }
  }

  data = malloc(s);
  p = data;

  /* Fill data */
  for(i = 0; i < count/4; i++) {
    memcpy(p, &(gfl[i].Size), sizeof(dword));
    p++;
    if(gfl[i].Size > 0) {
      memcpy(p, gfl[i].Data, sizeof(dword) * gfl[i].Size);
      p += gfl[i].Size;
    }

    if(gfl[i].last) {
      break;
    }
  }

  printf("ListIndices: Written %d lists into %d(%d) bytes\n", i, (void *)p - data,
         sizeof(char) * 16 * count);
  *size = (void *)p - data;
  return data;
}

void PrepareDataWrite(GFF_HEADER *header) {

  /* Check and free previos data */
  if(header->StructsBlob != NULL) {
    free(header->StructsBlob);
    header->StructsBlob = NULL;
  }
  if(header->FieldsBlob != NULL) {
    free(header->FieldsBlob);
    header->FieldsBlob = NULL;
  }
  if(header->LabelsBlob != NULL) {
    free(header->LabelsBlob);
    header->LabelsBlob = NULL;
  }
  if(header->ListIndiciesBlob != NULL) {
    free(header->ListIndiciesBlob);
    header->ListIndiciesBlob = NULL;
  }

  /* Create blobs from structured data */
  header->StructsBlob = MakeStructsBlob(&(header->StructBlobSize), header->Structs,
                                        header->StructCount);
  header->FieldsBlob = MakeFieldsBlob(&(header->FieldsBlobSize), header->Fields,
                                      header->FieldCount);
  header->LabelsBlob = MakeLabelsBlob(&(header->LabelsBlobSize), header->Labels,
                                      header->LabelCount);
  header->ListIndiciesBlob = MakeListIndicesBlob(&(header->ListIndiciesBlobSize),
                                                 header->ListIndicies,
                                                 header->ListIndiciesCount);

  /* Calculate data blocks sizes */
  header->StructCount = header->StructBlobSize / 3 / sizeof(dword); // 3 elements in each struct;
  header->FieldCount = header->FieldsBlobSize / 3 / sizeof(dword); // 3 elements in each field;
  header->LabelCount = header->LabelsBlobSize / 16; // 16chars each label;
  /* No intensions to change sizeof of FieldData and FieldIndicies*/
  header->ListIndiciesCount = header->ListIndiciesBlobSize;

  /* Calculate offsets */
  header->StructOffset        = GFF_HEADER_SIZE;
  header->FieldOffset         = GFF_HEADER_SIZE + header->StructBlobSize;
  header->LabelOffset         = GFF_HEADER_SIZE + header->StructBlobSize +
                                header->FieldsBlobSize;
  header->FieldDataOffset     = GFF_HEADER_SIZE + header->StructBlobSize +
                                header->FieldsBlobSize + header->LabelsBlobSize;
  header->FieldIndiciesOffset = GFF_HEADER_SIZE + header->StructBlobSize +
                                header->FieldsBlobSize + header->LabelsBlobSize +
                                header->FieldDataCount;
  header->ListIndiciesOffset  = GFF_HEADER_SIZE + header->StructBlobSize +
                                header->FieldsBlobSize + header->LabelsBlobSize +
                                header->FieldDataCount + header->FieldIndiciesCount;
}

int WriteDownFile(const char *filename, GFF_HEADER *header) {
  FILE *f;

  f = fopen(filename, "wb");
  if (!f) {
    printf("Unable to open file!");
    return 1;
  }

  /* write header */
  fwrite(header->FileType, 4, 1, f);
  fwrite(header->FileVersion, 4, 1, f);
  fwrite(&(header->StructOffset), sizeof(dword), 1, f);
  fwrite(&(header->StructCount), sizeof(dword), 1, f);
  fwrite(&(header->FieldOffset), sizeof(dword), 1, f);
  fwrite(&(header->FieldCount), sizeof(dword), 1, f);
  fwrite(&(header->LabelOffset), sizeof(dword), 1, f);
  fwrite(&(header->LabelCount), sizeof(dword), 1, f);
  fwrite(&(header->FieldDataOffset), sizeof(dword), 1, f);
  fwrite(&(header->FieldDataCount), sizeof(dword), 1, f);
  fwrite(&(header->FieldIndiciesOffset), sizeof(dword), 1, f);
  fwrite(&(header->FieldIndiciesCount), sizeof(dword), 1, f);
  fwrite(&(header->ListIndiciesOffset), sizeof(dword), 1, f);
  fwrite(&(header->ListIndiciesCount), sizeof(dword), 1, f);

  /* Write data */
  fwrite(header->StructsBlob, sizeof(char), header->StructBlobSize, f);
  fwrite(header->FieldsBlob, sizeof(char), header->FieldsBlobSize, f);
  fwrite(header->LabelsBlob, sizeof(char), header->LabelsBlobSize, f);
  fwrite(header->FieldDataBlock, sizeof(char), header->FieldDataCount, f);
  fwrite(header->FieldIndices, sizeof(char), header->FieldIndiciesCount, f);
  fwrite(header->ListIndiciesBlob, sizeof(char), header->ListIndiciesBlobSize, f);


  fclose(f);
  return 0;
}
