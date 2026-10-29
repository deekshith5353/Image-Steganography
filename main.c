#include <stdio.h>
#include "encode.h"
#include "decode.h"
#include "types.h"
#include <string.h>
OperationType check_operation_type(char *);

int main(int argc, char *argv[]) // .aout -e <source_file> <secret_data_file> <optional>
{
    if (argc < 2)
    {
        fprintf(stderr, "ERROR: Missing operation type (-e or -d)\n");
        return 1;
    }

    //step 1 -> check check_operation_type(argv[1]) is returning e_encode or not
    // yes -> EncodeInfo encInfo;
    if(check_operation_type(argv[1]) == e_encode)
    {
        EncodeInfo encInfo;
        // check read_and_validate_encode_args(argv, &encInfo) is returning e_success or e_failure
        // failure -> print error msg and stop
        // success -> print success msg 
            if (argc < 4 || argc > 5 || read_and_validate_encode_args(argv, &encInfo) != e_success)
            {
                fprintf(stderr, "ERROR: Invalid arguments\n");
                return 1;
            }
            else
            {
                printf("Arguments validated successfully\n");
            }
            // check do_encoding(&encInfo) is returning e_success or e_failure
            // failure -> print error msg and stop
            // success -> print success msg and stop
            if(do_encoding(&encInfo) != e_success)
            {
                fprintf(stderr, "ERROR: Encoding failed\n");
                return 1;
            }
            else
            {
                printf("Encoding completed successfully\n");
            }
    }
    else if(check_operation_type(argv[1]) == e_decode)
    {
        DecodeInfo decInfo;
        // check read_and_validate_decode_args(argv, &decInfo) is returning e_success or e_failure
        // failure -> print error msg and stop
        // success -> print success msg 
            if (argc < 3 || argc > 4 || read_and_validate_decode_args(argv, &decInfo) != d_success)
            {
                fprintf(stderr, "ERROR: Invalid arguments\n");
                return 1;
            }
            else
            {
                printf("Arguments validated successfully\n");
            }
            // check do_decoding(&decInfo) is returning e_success or e_failure
            // failure -> print error msg and stop
            // success -> print success msg and stop
            if(do_decoding(&decInfo) != d_success)
            {
                fprintf(stderr, "ERROR: Decoding failed\n");
                return 1;
            }
            else
            {
                printf("Decoding completed successfully\n");
            }
    }
    else
    {
        fprintf(stderr, "ERROR: Unsupported operation type\n");
        return 1;
    }

    return 0;
}

OperationType check_operation_type(char *symbol)
{
    //step 1 -> check symbol is -e or not
    // yes -> return e_encode
    if(strcmp(symbol, "-e") == 0)
    {
        return e_encode;
    }
    
    // step 2 -> check symbol is -d or not
        // yes -> return e_decode
    if(strcmp(symbol, "-d") == 0)
    {
        return e_decode;
    }

    // return e_unsupported
    return e_unsupported;
}
