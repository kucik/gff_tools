
#ifndef __GFF_WRITER
#  define __GFF_WRITER

# ifdef __cplusplus
extern "C" {
# endif /* __cplusplus */

#include "gffTypes.h"

/**
 * Create blob in memory of structs to be written into file
 */
void *MakeStructsBlob(int *size, gff_struct *gfs, int count);

/**
 * Create blob in memory of Fields to be written into file
 */
void *MakeFieldsBlob(int *size, gff_field *gfs, int count);

void *MakeLabelsBlob(int *size, gff_label *gfl, int count);

void *MakeListIndicesBlob(int *size, gff_list *gfl, int count);

/**
 * Prepare header to be written into file
 */
void PrepareDataWrite(GFF_HEADER *header);

int WriteDownFile(const char *filename, GFF_HEADER *header);


/* Reader */
GFF_HEADER *ReadGffFile(const char *filename);

void PrintHeaderCheck(GFF_HEADER *header);

# ifdef __cplusplus
}
# endif /* __cplusplus */


#endif  /* __GFF_WRITER */
