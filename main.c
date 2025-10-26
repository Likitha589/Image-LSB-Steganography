/*DOCUMENTATION:
Name : Pasam Likitha
Description : LSB Image Steganography project to hide the contents of the secret message in an image file
Date : 24-09-25
*/
#include <stdio.h>
#include "encode.h"
#include "types.h"
#include <string.h>
#include "decode.h"
//Color Macros
#define RESET   "\033[0m"
#define RED     "\033[1;31m"
#define GREEN   "\033[1;32m"
#define MAGENTA "\033[1;35m"
#define WHITE   "\033[1;37m"

//Step 1: check the argv is -e or not if yes Goto step 2
//Step 2: return e_encode(0)

//Step 3: check if argv is -d or not, if yes Goto step 4
//Step 4: return e_decode(1)

//Step 5: return e_unsupported(2)

OperationType check_operation_type(int argc,char *argv[])
//we write the function definition in main becaouse we have to check both encoding & decoding
{
    if(strcmp(argv[1],"-e")==0)
    return e_encode;
    else if(strcmp(argv[1],"-d")==0)
    return e_decode;
    else
    return e_unsupported;
}

int main(int argc,char *argv[])
{

    if(argc<2)
    {
        fprintf(stderr,RED "INFO : Please pass valid Arguments\n"RESET);
        fprintf(stderr,RED "INFO : Encoding - minimum 4 arguments\n"RESET);
        fprintf(stderr,RED "INFO : Decoding - minimum 3 arguments\n"RESET);

        return 1;
    }

    OperationType op_type=check_operation_type(argc,argv);//argv[1]
   
    if(op_type==e_encode)
    {
        printf(MAGENTA "INFO: ## Selected Encoding ##\n"RESET);
        EncodeInfo encInfo;

        if(read_and_validate_encode_args(argc,argv,&encInfo)==e_failure)
        {
            fprintf(stderr,RED"ERROR: Validation failed\n"RESET);
            return 1;
        }
        else
        {
            printf(GREEN"INFO: Read and validate executed successfully\n"RESET);
            if(do_encoding(&encInfo)==e_success)//call encoding function
            {
                 //print encoding msg success
                 printf(GREEN"INFO: ## Encoding completed successfully ##\n"RESET);
            }
            else
            {
                //print error msg
                fprintf(stderr,RED"ERROR: Encoding failed\n"RESET);
                return 1;
            }
        }
    }
    else if(op_type==e_decode)
    {
        printf(MAGENTA"INFO: ## Selected Decoding ##\n" RESET);
        DecodeInfo decInfo;

        if(read_and_validate_decode_args(argc,argv,&decInfo)==e_failure)
        {
                fprintf(stderr,RED"ERROR: Validation failed for decoding\n");
                return 1;
        }
        else
        {
            printf(GREEN"INFO: Read and validate executed successfully\n"RESET);
            if(open_decode_files(&decInfo)==e_failure)
           {
                fprintf(stderr,RED"ERROR: File opening failed for decoding\n"RESET);
                return 1;
            }

            if(do_decoding(&decInfo)==e_failure)
            {
                fprintf(stderr,RED"ERROR : Decoding failed\n"RESET);
                return 1;
            }
    }
    }
    else
    {
            fprintf(stderr, RED"ERROR: Unsupported operation. Use -e for encode or -d for decode\n"RESET);
        return 1;
    }


    return 0;
}