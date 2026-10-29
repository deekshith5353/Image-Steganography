#include<stdio.h>
#include<stdlib.h>
#include "decode.h"
#include "types.h"
#include <string.h>
#include "common.h"
Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo)
{
    // step 1 -> check argv[2] is having .bmp or not
    if (strstr(argv[2], ".bmp") != NULL)
    {
        decInfo->stego_image_fname = argv[2];
    }
    else
    {   
        return d_failure;
    }
    // step 2 -> check argv[3] is having extention
    if(argv[3]!= NULL)
    {char* ptr=NULL;
    if (ptr=strchr(argv[3], ".") != NULL)
    {   
        *ptr='\0';
    }
    decInfo->secret_fname = argv[3];
    return d_success;
    }
    else
    {
        decInfo->secret_fname = "stego_d";
    }
    return d_success;
}

Status open_decode_files(DecodeInfo *decInfo)
{
    // Stego Image file
    decInfo->fptr_stego_image = fopen(decInfo->stego_image_fname, "rb");
    // Do Error handling
    if (decInfo->fptr_stego_image == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR:file %s not found\n", decInfo->stego_image_fname);
        return d_failure;
    }
    if (fseek(decInfo->fptr_stego_image, 54, SEEK_SET) != 0)
    {
        fprintf(stderr, "ERROR: Unable to seek to BMP pixel data\n");
        return d_failure;
    }
    if(decode_magic_string(decInfo) != d_success)
    {
        fprintf(stderr, "ERROR: Unable to find magic string\n");
        return d_failure;
    }
    else
    {
        printf("Magic string found successfully\n");
    }
    if(decode_secret_file_extn_size(decInfo) != d_success)
    {
        fprintf(stderr, "ERROR: Unable to decode secret file extension size\n");
        return d_failure;
    }
    else
    {
        printf("Secret file extension size decoded successfully\n");
    }
    // Secret file
    if (decInfo->extn_secret_file_size <= 0 ||
        decInfo->extn_secret_file_size >= (int)sizeof(decInfo->extn_secret_file) ||
        decode_secret_file_extn(decInfo->extn_secret_file_size, decInfo) != d_success)
    {
        fprintf(stderr, "ERROR: Unable to decode secret file extension\n");
        return d_failure;
    }
    else
    {
        printf("Secret file extension decoded successfully\n");
    }
    char filename[100];
    if (strlen(decInfo->secret_fname) + strlen(decInfo->extn_secret_file) >= sizeof(filename))
    {
        fprintf(stderr, "ERROR: Output filename is too long\n");
        return d_failure;
    }
    strcpy(filename, decInfo->secret_fname);
    strcat(filename, decInfo->extn_secret_file);
    decInfo->fptr_secret = fopen(filename, "wb");
    // Do Error handling
    if (decInfo->fptr_secret == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", decInfo->secret_fname);

        return d_failure;
    }
    return d_success;
}
Status decode_magic_string(DecodeInfo *decInfo)
{   char magic[strlen(MAGIC_STRING)+1];
    for (size_t i = 0; i < strlen(MAGIC_STRING); i++)
    { 
        magic[i] = 0;
        char buffer[8];
        if (fread(buffer, 1, sizeof(buffer), decInfo->fptr_stego_image) != sizeof(buffer))
        {
            return d_failure;
        }
        for(int j=0;j<8;j++)
        {
        magic[i]|=((buffer[j]&1)<< (7-j));
        }
    }
    magic[strlen(MAGIC_STRING)] = '\0';
    if(strcmp(magic,MAGIC_STRING)==0)
    {
     return d_success;
    }
    else
    {
        return d_failure;
    }
}
Status decode_secret_file_extn_size(DecodeInfo *decInfo)
{
    char buffer[32];
    //step 1 -> read 32 bytes from stego file
    if (fread(buffer, 1, sizeof(buffer), decInfo->fptr_stego_image) != sizeof(buffer))
    {
        return d_failure;
    }
    //step 2 -> call decode_byte_from_lsb(buffer) and store it into decInfo -> extn_secret_file
    uint size=0;
    for(int i = 0; i < 32; i++)
    { 
        size |= (uint)(buffer[i] & 1) << (31 - i);
    }
    decInfo->extn_secret_file_size=size;
    return d_success;
}

Status decode_secret_file_extn(int size,DecodeInfo *decInfo)
{
  for(int i=0;i<size;i++)
  { 
    decInfo->extn_secret_file[i] = 0;
    char buffer[8];
    if (fread(buffer, 1, sizeof(buffer), decInfo->fptr_stego_image) != sizeof(buffer))
    {
        return d_failure;
    }
    for(int j=0;j<8;j++)
    {
       decInfo->extn_secret_file[i]|=((buffer[j]&1)<< (7-j));
    }
  }
   decInfo->extn_secret_file[size] = '\0';
   return d_success;

}

Status decode_secret_file_size(DecodeInfo *decInfo)
{
    char buffer[32];
    //step 1 -> read 32 bytes from stego file
    if (fread(buffer, 1, sizeof(buffer), decInfo->fptr_stego_image) != sizeof(buffer))
    {
        return d_failure;
    }
    //step 2 -> call decode_byte_from_lsb(buffer) and store it into decInfo -> extn_secret_file
    uint size=0;
    for(int i = 0; i < 32; i++)
    { 
        size |= (uint)(buffer[i] & 1) << (31 - i);
    }
    decInfo->size_secret_file=size;
    return d_success;
}

Status decode_secret_file_data(uint size,DecodeInfo *decInfo)
{
    for (uint i = 0; i < size; i++)
  { 
    char buffer[8];
    char ch=0;
    fread(buffer,8,1,decInfo->fptr_stego_image);
    for(int j=0;j<8;j++)
    {
       ch|=((buffer[j]&1)<< (7-j));
    }
    if (fwrite(&ch, 1, 1, decInfo->fptr_secret) != 1)
    {
        return d_failure;
    }
  }
   return d_success;

}

Status do_decoding(DecodeInfo *decInfo)
{
    if(open_decode_files(decInfo) != d_success)
    {
        return d_failure;
    }
    if(decode_secret_file_size(decInfo) != d_success)
    {
        fprintf(stderr, "ERROR: Unable to decode secret file size\n");
        return d_failure;
    }
    else
    {
        printf("Secret file size decoded successfully\n");
    }
    long data_start = ftell(decInfo->fptr_stego_image);
    if (data_start < 0 || fseek(decInfo->fptr_stego_image, 0, SEEK_END) != 0)
    {
        fprintf(stderr, "ERROR: Unable to validate secret file size\n");
        return d_failure;
    }
    long remaining = ftell(decInfo->fptr_stego_image) - data_start;
    if (remaining < 0 || decInfo->size_secret_file > (uint)remaining / 8)
    {
        fprintf(stderr, "ERROR: Secret file size exceeds image data\n");
        return d_failure;
    }
    if (fseek(decInfo->fptr_stego_image, data_start, SEEK_SET) != 0)
    {
        return d_failure;
    }
    if(decode_secret_file_data(decInfo->size_secret_file,decInfo) != d_success)
    {
        fprintf(stderr, "ERROR: Unable to decode secret file data\n");
        return d_failure;
    }
    else
    {
        printf("Secret file data decoded successfully\n");
    }
    fclose(decInfo->fptr_stego_image);
    fclose(decInfo->fptr_secret);
    return d_success;
}