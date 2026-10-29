#ifndef DECODE_H
#define DECODE_H

#include "types.h"
typedef struct _DecodeInfo
{
    char *stego_image_fname;
    FILE *fptr_stego_image;  
    uint image_capacity;   
    char *secret_fname;       
    FILE *fptr_secret;  
    int  extn_secret_file_size;  
    char extn_secret_file[5];    
    uint size_secret_file;   

} DecodeInfo;

// Function prototypes for decoding operations
Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo);
Status do_decoding(DecodeInfo *decInfo);
Status open_decode_files(DecodeInfo *decInfo);
Status decode_magic_string(DecodeInfo *decInfo);
Status decode_secret_file_extn_size(DecodeInfo *decInfo);
Status decode_secret_file_extn(int size,DecodeInfo *decInfo);
Status decode_secret_file_size(DecodeInfo *decInfo);
Status decode_secret_file_data(uint size,DecodeInfo *decInfo);
#endif // DECODE_H