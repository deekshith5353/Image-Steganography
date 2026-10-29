#include <stdio.h>
#include "encode.h"
#include "types.h"
#include <string.h>
#include "common.h"
/* Function Definitions */

/* Get image size
 * Input: Image file ptr
 * Output: width * height * bytes per pixel (3 in our case)
 * Description: In BMP Image, width is stored in offset 18,
 * and height after that. size is 4 bytes
 */
uint get_image_size_for_bmp(FILE *fptr_image)
{
    uint width, height;
    // Seek to 18th byte
    fseek(fptr_image, 18, SEEK_SET);

    // Read the width (an int)
    fread(&width, sizeof(int), 1, fptr_image);
    printf("width = %u\n", width);

    // Read the height (an int)
    fread(&height, sizeof(int), 1, fptr_image);
    printf("height = %u\n", height);

    // Return image capacity
    return width * height * 3;
}

uint get_file_size(FILE *fptr)
{
    // Find the size of secret file data
    fseek(fptr, 0, SEEK_END);
    uint size = ftell(fptr);
    // return secret file size
    rewind(fptr);
    return size;
}

/*
 * Get File pointers for i/p and o/p files
 * Inputs: Src Image file, Secret file and
 * Stego Image file
 * Output: FILE pointer for above files
 * Return Value: e_success or e_failure, on file errors
 */

Status read_and_validate_encode_args(char *argv[], EncodeInfo *encInfo)
{
    // step 1 -> check argv[2] is having .bmp or not
    if (strstr(argv[2], ".bmp") != NULL)
    {
        encInfo->src_image_fname = argv[2];
    }
    else
    {
        return e_failure;
    }

    // step 2 -> check argv[3] is having extn there or not
    if (strstr(argv[3], ".") != NULL)
    {
        encInfo->secret_fname = argv[3];
    }
    else
    {
        return e_failure;
    }
    char *dot = strrchr(argv[3], '.');
    if (dot != NULL && strlen(dot) < sizeof(encInfo->extn_secret_file))
    {
        strcpy(encInfo->extn_secret_file, dot);
    }
    else
    {
        return e_failure;
    }

    // step 3 -> check argv[4] is NULL or not
    if (argv[4] != NULL)
    {
        if (strstr(argv[4], ".bmp") != NULL)
        {
            encInfo->stego_image_fname = argv[4];
        }
        else
        {
            return e_failure;
        }
    }
    else
    {
        encInfo->stego_image_fname = "stego_e.bmp";
    }
    return e_success;// return e_success;

}

Status open_files(EncodeInfo *encInfo)
{
    // Src Image file
    encInfo->fptr_src_image = fopen(encInfo->src_image_fname, "rb");
    // Do Error handling
    if (encInfo->fptr_src_image == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->src_image_fname);

        return e_failure;
    }

    // Secret file
    encInfo->fptr_secret = fopen(encInfo->secret_fname, "rb");
    // Do Error handling
    if (encInfo->fptr_secret == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->secret_fname);

        return e_failure;
    }

    // Stego Image file
    encInfo->fptr_stego_image = fopen(encInfo->stego_image_fname, "wb");
    // Do Error handling
    if (encInfo->fptr_stego_image == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->stego_image_fname);

        return e_failure;
    }

    // No failure return e_success
    return e_success;
}

Status check_capacity(EncodeInfo *encInfo)
{
    //step 1 -> encInfo -> image_capacity = call get_image_size_for_bmp(encoInfo -> fptr_src_image)
    encInfo->image_capacity = get_image_size_for_bmp(encInfo->fptr_src_image);

    //step 2 -> encInfo -> size_secret_file = call get_file_size(encInfo -> fptr_secret)
    encInfo->size_secret_file = get_file_size(encInfo->fptr_secret);

    //step 3 -> check encInfo -> image_capacity > 16 + 32 + (strlen(encInfo -> extn_secret_file) * 8) + 32 + (encInfo -> size_secret_file * 8)
                //t -> return e_success;
                //f -> return e_failure
    if (encInfo->image_capacity > 16 + 32 + (strlen(encInfo->extn_secret_file) * 8) + 32 + (encInfo->size_secret_file * 8))
    {
        return e_success;
    }
    else
    {
        return e_failure;
    }
}

Status copy_bmp_header(FILE *fptr_src_image, FILE *fptr_dest_image)
{   char buffer[54];
    // step 1 -> rewind source file pointer
    rewind(fptr_src_image);

    // step 2 -> read 54 bytes from source file
    fread(buffer, 54, 1, fptr_src_image);
    // step 3 -> write 54 bytes to dest file
    fwrite(buffer, 54, 1, fptr_dest_image);

    // return e_success
    return e_success;
}
Status encode_magic_string(const char *magic_string, EncodeInfo *encInfo)
{
    for (size_t i = 0; i < strlen(magic_string); i++)
    {char buffer[8];
    //step 1 -> read 8 bytes from source file
    fread(buffer, 8, 1, encInfo->fptr_src_image);
    //step 2 -> call encode_byte_to_lsb(magic_string[i], buffer)
    encode_byte_to_lsb(magic_string[i], buffer);
    //step 3 -> write the buffer into dest file
    fwrite(buffer, 8, 1, encInfo->fptr_stego_image);
    }// repeat this for strlen(magic_string) times from step 1
    return e_success;
}
Status encode_secret_file_extn_size(int size, EncodeInfo *encInfo)
{
    char buffer[32];
    //step 1 -> read 32 bytes from source file
    fread(buffer, 32, 1, encInfo->fptr_src_image);
    //step 2 -> call encode_size_to_lsb(size, buffer)
    encode_size_to_lsb(size, buffer);
    //step 3 -> write the buffer into dest file
    fwrite(buffer, 32, 1, encInfo->fptr_stego_image);
    return e_success;
}

Status encode_secret_file_extn(const char *file_extn, EncodeInfo *encInfo)
{   
    for(size_t i = 0; i < strlen(file_extn); i++)
    {   char buffer[8];
        //step 1 -> read 8 bytes from source file
        fread(buffer, 8, 1, encInfo->fptr_src_image);
        //step 2 -> call encode_byte_to_lsb(file_extn[i], buffer)
        encode_byte_to_lsb(file_extn[i], buffer);
        //step 3 -> write the buffer into dest file
        fwrite(buffer, 8, 1, encInfo->fptr_stego_image);
    } // repeat this for strlen(file_extn) times from step 1
    return e_success;
}

Status encode_secret_file_size(long file_size, EncodeInfo *encInfo)
{
    // char buffer[32];
    char buffer[32];
    //step 1 -> read 32 bytes from source file
    fread(buffer, 32, 1, encInfo->fptr_src_image);
    //step 2 -> call encode_size_to_lsb(file_size, buffer)
    encode_size_to_lsb(file_size, buffer);
    //step 3 -> write the buffer into dest file
    fwrite(buffer, 32, 1, encInfo->fptr_stego_image);
    return e_success;
}

Status encode_secret_file_data(EncodeInfo *encInfo)
{   
    //step 1 -> read secret data from file and store it into encInfo -> secret_data
    for(int i = 0; i < encInfo->size_secret_file; i++)
    {   char ch;
        fread(&ch, 1, 1, encInfo->fptr_secret);
        char buffer[8];
    //step 2 -> read 8 bytes from source file
        fread(buffer, 8, 1, encInfo->fptr_src_image);
    //step 3 -> call encode_byte_to_lsb(encInfo -> secret_data[i], buffer)
        encode_byte_to_lsb(ch, buffer);
    //step 4 -> write the buffer into dest file
        fwrite(buffer, 8, 1, encInfo->fptr_stego_image);
    }// repeat this for strlen(encInfo -> size_secret_file) times from step 2
    return e_success;
}

Status copy_remaining_img_data(FILE *fptr_src, FILE *fptr_dest)
{
    char buffer[4096];
    size_t bytes_read;

    while ((bytes_read = fread(buffer, 1, sizeof(buffer), fptr_src)) > 0)
    {
        if (fwrite(buffer, 1, bytes_read, fptr_dest) != bytes_read)
        {
            return e_failure;
        }
    }

    return ferror(fptr_src) ? e_failure : e_success;
}

Status encode_byte_to_lsb(char data, char *image_buffer)
{
    for (int index = 0; index < 8; index++)
    {
        image_buffer[index] = image_buffer[index] & 0xFE;//11111110
        int mask = (data >> (7 - index)) & 1;
        image_buffer[index] = image_buffer[index] | mask;
    }
    return e_success;
}

Status encode_size_to_lsb(int size, char *image_buffer)
{
    for (int index = 0; index < 32; index++)
    {
        image_buffer[index] = image_buffer[index] & 0xFE;//11111110
        int mask = (size >> (31 - index)) & 1;
        image_buffer[index] = image_buffer[index] | mask;
    }
    return e_success;
}

Status do_encoding(EncodeInfo *encInfo)
{
    //step 1 -> check open_files(encInfo) is returning e_success or not
            //no -> print error return e_failure
            //yes -> print success and goto next step
    if(open_files(encInfo) != e_success)
    {
        fprintf(stderr, "ERROR: Unable to open files\n");
        return e_failure;
    }
    else
    {
        printf("Files opened successfully\n");
    }
    //step 2 -> check check_capacity(encInfo) is returning e_success or not
            //no -> print error return e_failure
            //yes -> print success and goto next step
    if(check_capacity(encInfo) != e_success)
    {
        fprintf(stderr, "ERROR: Insufficient image capacity\n");
        return e_failure;
    }
    else
    {
        printf("Image has sufficient capacity\n");
    }
    //step 3 -> call copy_bmp_header(encInfo -> fptr_src_image, encInfo -> fptr_stego_image)is returning e_success or not
            //no -> print error return e_failure
            //yes -> print success and goto next step
    if(copy_bmp_header(encInfo->fptr_src_image, encInfo->fptr_stego_image) != e_success)
    {
        fprintf(stderr, "ERROR: Unable to copy BMP header\n");
        return e_failure;
    }
    else
    {
        printf("BMP header copied successfully\n");
    }
    //step 4 -> call encode_magic_string(MAGIC_STRING, encInfo)
    if(encode_magic_string(MAGIC_STRING, encInfo) != e_success)
    {
        fprintf(stderr, "ERROR: Unable to encode magic string\n");
        return e_failure;
    }
    else
    {
        printf("Magic string encoded successfully\n");
    }
    //step 5 -> call encode_secret_file_extn_size(strlen(encInfo -> extn_secret_file), encInfo)
    if(encode_secret_file_extn_size(strlen(encInfo->extn_secret_file), encInfo) != e_success)
    {
        fprintf(stderr, "ERROR: Unable to encode secret file extension size\n");
        return e_failure;
    }
    else
    {
        printf("Secret file extension size encoded successfully\n");
    }
    //step 6 -> call encode_secret_file_extn(encInfo -> extn_secret_file, encInfo)
    if(encode_secret_file_extn(encInfo->extn_secret_file, encInfo) != e_success)
    {
        fprintf(stderr, "ERROR: Unable to encode secret file extension\n");
        return e_failure;
    }
    else
    {
        printf("Secret file extension encoded successfully\n");
    }
    //step 7 -> call encode_secret_file_size(encInfo ->  size_secret_file, encInfo)
    if(encode_secret_file_size(encInfo->size_secret_file, encInfo) != e_success)
    {
        fprintf(stderr, "ERROR: Unable to encode secret file size\n");
        return e_failure;
    }
    else
    {
        printf("Secret file size encoded successfully\n");
    }
    //step 8 -> call encode_secret_file_data(encInfo)
    if(encode_secret_file_data(encInfo) != e_success)
    {
        fprintf(stderr, "ERROR: Unable to encode secret file data\n");
        return e_failure;
    }
    else
    {
        printf("Secret file data encoded successfully\n");
    }
    //step 9 -> copy_remaining_img_data(encInfo -> fptr_src_image, encInfo -> fptr_stego_image)is returning e_success or not
            //no -> print error return e_failure
            //yes -> print success
    if(copy_remaining_img_data(encInfo->fptr_src_image, encInfo->fptr_stego_image) != e_success)
    {
        fprintf(stderr, "ERROR: Unable to copy remaining image data\n");
        return e_failure;
    }
    else
    {
        printf("Remaining image data copied successfully\n");
    }

    fclose(encInfo->fptr_src_image);
    fclose(encInfo->fptr_secret);
    fclose(encInfo->fptr_stego_image);

    // return e_success
    return e_success;

}