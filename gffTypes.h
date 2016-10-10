

#ifndef __GFF_TYPES
#  define __GFF_TYPES

# ifdef __cplusplus
extern "C" {
# endif /* __cplusplus */

  #define GFF_HEADER_SIZE 56

  typedef unsigned int dword;

  typedef char gff_byte;
  typedef char gff_char;
  typedef char gff_CResRef[16+1];
  typedef double gff_double;
  typedef unsigned int gff_dword;
  typedef unsigned long gff_dword64;
  typedef float gff_float;
  typedef int gff_int;
  typedef long gff_int64;
  typedef short gff_short;
  typedef unsigned short gff_word;

  typedef enum _GFF_LANGUAGE
        {
                LangEnglish            = 0,
                LangFrench             = 1,
                LangGerman             = 2,
                LangItalian            = 3,
                LangSpanish            = 4,
                LangPolish             = 5,
                LangKorean             = 128,
                LangChineseTraditional = 129,
                LangChineseSimplified  = 130,
                LangJapanese           = 131,
                LangLastGFFLanguage
        } GFF_LANGUAGE;


        //
        // Define the GFF on-disk file structures.  This data is based on the
        // BioWare Aurora engine documentation.
        //
        // http://nwn.bioware.com/developers/Bioware_Aurora_GFF_Format.pdf
        //

  typedef enum _GFF_FIELD_TYPE
        {
                GFF_BYTE          = 0,
                GFF_CHAR          = 1,
                GFF_WORD          = 2,
                GFF_SHORT         = 3,
                GFF_DWORD         = 4,
                GFF_INT           = 5,
                GFF_DWORD64       = 6,
                GFF_INT64         = 7,
                GFF_FLOAT         = 8,
                GFF_DOUBLE        = 9,
                GFF_CEXOSTRING    = 10,
                GFF_RESREF        = 11,
                GFF_CEXOLOCSTRING = 12,
                GFF_VOID          = 13,
                GFF_STRUCT        = 14,
                GFF_LIST          = 15,
                GFF_RESERVED      = 16,
                GFF_VECTOR        = 17,

                LAST_GFF_FIELD_TYPE

        } GFF_FIELD_TYPE;

  typedef struct _GFF_FIELD {
    GFF_FIELD_TYPE Type; // Data type
    dword LabelIndex;    // Index into the Label Array
    dword DataOrDataOffset;  // If Field.Type is a simple data type (see table below),
                         // then this is the value actual of the field.
                         // If Field.Type is a complex data type (see table below),
                         // then this is a byte offset into the Field Data block.

    char *Label;
    union {
      gff_byte    v_byte;
      gff_char    v_char;
      gff_word    v_word;
      gff_short   v_short;
      gff_dword   v_dword;
      gff_int     v_int;
      gff_dword64 v_dword64;
      gff_int64   v_int64;
      gff_float   v_float;
      gff_double  v_double;
      char       *v_resref;
      char       *v_cexostring;
      struct _GFF_STRUCT *v_struct;
      struct _GFF_LIST   *v_list;
    };
  } gff_field;
  typedef gff_field *gff_field_p;


  typedef struct _GFF_STRUCT {
    dword Type;              // Progremmer-defined integer ID.
    dword DataOrDataOffset;  // If Struct.FieldCount = 1, this is an index into the
                             // Field Array.
                             // If Struct.FieldCount > 1, this is a byte offset into
                             // the Field Indices array, where there is an array of
                             // DWORDs having a number of elements equal to
                             // Struct.FieldCount. Each one of these DWORDs is an
                             // index into the Field Array.
    dword FieldCount;        // Number of fields in this Struct.

    // library routine values
    gff_field_p  *fields;      // Array of Referenced Fields. Size is FieldCount.

  } gff_struct;
  typedef gff_struct *gff_struct_p;

  typedef char gff_label[16+1];


  typedef struct _GFF_LIST {
    dword Size;
    dword *Data;
    // Support value
    dword __offset;
    gff_struct_p *structs;
    short last;
  } gff_list;

  typedef struct _GFF_HEADER {
    /* File header fields */
    char FileType[4+1];            // "GFF"
    char FileVersion[4+1];         // "V3.2"
    dword StructOffset;            // Offset of Struct array as bytes from the
                                   // beginning of the file
    dword StructCount;             // Number of elements in Struct array
    dword FieldOffset;             // Offset of Field array as bytes from the
                                   // beginning of the file
    dword FieldCount;              // Number of elements in Field array
    dword LabelOffset;             // Offset of Label array as bytes from the
                                   //beginning of the file
    dword LabelCount;              // Number of elements in Label array
    dword FieldDataOffset;         // Offset of Field Data as bytes from the
                                   // beginning of the file
    dword FieldDataCount;          // Number of bytes in Field Data block
    dword FieldIndiciesOffset;     // Offset of Field Indicies array as bytes from
                                   // the beginning of the file
    dword FieldIndiciesCount;      // Number of bytes in Field Indicies array
    dword ListIndiciesOffset;      // Offset of List Indicies array as bytes from the
                                   // beginning of the file
    dword ListIndiciesCount;       // Number of bytes in List Indicies array

    /* Parsed data representation structs */
    gff_struct *Structs;
    gff_field  *Fields;
    gff_label  *Labels;
    char       *FieldDataBlock;
    dword      *FieldIndices;
    gff_list   *ListIndicies;

    /* Blob data format to be written into file*/
    void *StructsBlob;
    int StructBlobSize;
    void *FieldsBlob;
    int FieldsBlobSize;
    void *LabelsBlob;
    int LabelsBlobSize;
    // No need for FieldDataBlock and FieldIndices conversion
    void *ListIndiciesBlob;
    int ListIndiciesBlobSize;


  } GFF_HEADER;



# ifdef __cplusplus
}
# endif /* __cplusplus */

#endif  /* __GFF_TYPES */
