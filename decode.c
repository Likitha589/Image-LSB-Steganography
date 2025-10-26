#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "decode.h"
#include "common.h"
#include "types.h"

//Color Macros
#define RESET   "\033[0m"
#define RED     "\033[1;31m"
#define GREEN   "\033[1;32m"
#define MAGENTA "\033[1;35m"
#define WHITE   "\033[1;37m"

/*Function Definitions*/

// Function: read_and_validate_decode_args
// Purpose : Validate command line arguments for decoding
// Input   : argc (argument count), argv (argument values), decInfo structure
// Output  : e_success if valid, e_failure otherwise
Status read_and_validate_decode_args(int argc,char *argv[],DecodeInfo *decInfo)
{
        if(argc<3) // Need at least program name + option + stego image
        {
                fprintf(stderr,RED"ERROR: Not enough arguments for decoding\n"RESET);
                return e_failure;
        }

        // Validate that the 2nd argument (stego image) is a .bmp file
        if(strstr(argv[2],".bmp")==NULL)
        {
                fprintf(stderr,RED"ERROR: Stego image must be a BMP file\n"RESET);
                return e_failure;
        }

        // Store the stego image filename
        decInfo->stego_image_fname=argv[2];
        
        // If output filename is provided, store it, else keep NULL for now
        if(argc>3)
        {
                decInfo->output_fname=strdup(argv[3]);
        }
        else
        {
                decInfo->output_fname=NULL;
        }

        return e_success;
}

// Function: open_decode_files
// Purpose : Open the stego image file for reading
//AFTER DECODING EXTENSION WE HAVE TO OPEN SECRET FILE
Status open_decode_files(DecodeInfo *decInfo)
{
    decInfo->fptr_stego_image = fopen(decInfo->stego_image_fname, "rb");

    if (decInfo->fptr_stego_image == NULL)
    {
        perror(RED"ERROR: ");
        fprintf(stderr, "ERROR: Unable to open stego image %s\n", decInfo->stego_image_fname);
        return e_failure;
    }

    return e_success;
}

// Function: do_decoding
// Purpose : Perform the full decoding procedure step by step
Status do_decoding(DecodeInfo *decInfo)
{
        printf(MAGENTA "INFO: ## Decoded Procedure Started ##\n"RESET);

        // Skip 54-byte BMP header
        printf(WHITE"INFO: Opening stego image file\n"RESET);
        if(fseek(decInfo->fptr_stego_image,54,SEEK_SET)!=0)
        {
                fprintf(stderr,RED"ERROR: fseek failed on stego image\n"RESET);
                return e_failure;
        }
        printf(GREEN"INFO: Done. Image header skipped\n"RESET);
        
        // Decode and check magic string
        printf(WHITE"INFO: Checking Magic String Signature\n"RESET);
        if(decode_magic_string(decInfo)==e_failure)
        {
                fprintf(stderr,RED"ERROR: Magic String mismatch\n"RESET);
                return e_failure;
        }
        printf(GREEN"INFO: Magic String verified successfully\n"RESET);

        // Decode extension size
        printf(WHITE"INFO: Decoding Secret File Extension Size\n"RESET);
        if(decode_secret_file_extn_size(decInfo)==e_failure)
        {
                fprintf(stderr,RED"ERROR: Failed to decode secret file extension size\n"RESET);
                return e_failure;
        }
        printf(GREEN"INFO: Done. Extension size + %d\n"RESET,decInfo->extn_size);

        // Decode file extension (e.g., .txt, .c, .jpg)
        printf(WHITE"INFO: Decoding Secret File Extension\n"RESET);
        if(decode_secret_file_extn(decInfo)==e_failure)
        {
                fprintf(stderr,RED"ERROR: Failed to decode secret file extension\n"RESET);
                return e_failure;
        }
        printf(GREEN"INFO: Done. File will be saved as %s\n"RESET,decInfo->output_fname);
        
        // Open output file for writing decoded secret
        decInfo->fptr_output=fopen(decInfo->output_fname,"wb");
        if(!decInfo->fptr_output)
        {
                perror(RED"ERROR: ");
                fprintf(stderr,"ERROR: Unable to create output file %s\n"RESET,decInfo->output_fname);
                return e_failure;
        }
        
        // Decode size of secret file
        printf(WHITE"INFO: Decoding Secret File Size\n"RESET);
        if(decode_secret_file_size(decInfo)==e_failure)
        {
                fprintf(stderr,RED"ERROR: Failed to decode file size\n"RESET);
                return e_failure;
        }
        printf(GREEN"INFO: Done. Secret file size: %ld bytes\n"RESET,decInfo->size_secret_file);

        // Decode actual secret data
        printf(WHITE"INFO: Decoding Secret File Data\n"RESET);
        if(decode_secret_file_data(decInfo)==e_failure)
        {
                fprintf(stderr,RED"ERROR: Failed to decode secret file data\n"RESET);
                return e_failure;
        }
        printf(GREEN"INFO: Done. Secret file written successfully\n"RESET);

        printf(GREEN"INFO: ## Decoding Completed Successfully! ##\n"RESET);
        printf(WHITE"INFO: Output File saved as %s\n"RESET,decInfo->output_fname);
        return e_success;
}

// Function: decode_magic_string
// Purpose : Extract and verify magic string to confirm valid encoding
Status decode_magic_string(DecodeInfo *decInfo)
{
        int len=strlen(MAGIC_STRING);
        char magic[len+1];//+1 for null

        if(decode_data_from_image(magic,len,decInfo->fptr_stego_image)==e_failure)
        return e_failure;

        magic[len]='\0';

        if(strncmp(magic,MAGIC_STRING,len)==0)
                return e_success;
        else
                return e_failure;
}

// Function: decode_secret_file_extn_size
// Purpose : Decode size of secret file extension (stored in 32 bits)
Status decode_secret_file_extn_size(DecodeInfo *decInfo)
{
    char buffer[32];
    if (fread(buffer, 1, 32, decInfo->fptr_stego_image) != 32)
        return e_failure;
    decInfo->extn_size = decode_size_from_lsb(buffer);
    if (decInfo->extn_size <= 0 || decInfo->extn_size > 7) return e_failure;

    return e_success;
}

// Function: decode_secret_file_extn
// Purpose : Decode file extension (like .txt, .c, .jpg)
Status decode_secret_file_extn(DecodeInfo *decInfo)
{
    if (decode_data_from_image(decInfo->extn_secret_file,decInfo->extn_size,decInfo->fptr_stego_image) == e_failure)
        return e_failure; 

    decInfo->extn_secret_file[decInfo->extn_size] = '\0';
    // If no output file name was passed, generate one

    if (decInfo->output_fname==NULL)
    {
        decInfo->output_fname=malloc(strlen("output_secret")+strlen(decInfo->extn_secret_file)+1);
        if(!decInfo->output_fname)
        {
                fprintf(stderr,RED"ERROR: Memory allocation failed for output\n"RESET);
                return e_failure;
        }
        
        sprintf(decInfo->output_fname,"output_secret%s", decInfo->extn_secret_file);
        printf(WHITE"INFO: Output file not mentioned. Created <%s> as default\n"RESET,decInfo->output_fname);
    }
    else
    {
        char *dot=strrchr(decInfo->output_fname,'.');
        int namelen;

        if(dot)
        namelen=dot-decInfo->output_fname;//upto dot
        else
        namelen=strlen(decInfo->output_fname);//full name if no dot

        char *name=malloc(namelen+strlen(decInfo->extn_secret_file)+1);
        if(!name)
        {
                fprintf(stderr,RED"ERROR: Memory allocation failed for output\n"RESET);
                return e_failure;
        }
        strncpy(name,decInfo->output_fname,namelen);
        name[namelen]='\0';
        strcat(name,decInfo->extn_secret_file);
        //string concatenation with extension and name
        decInfo->output_fname=name;
    }
    return e_success;
}

// Function: decode_secret_file_size
// Purpose : Decode secret file size (stored in 32 bits)
Status decode_secret_file_size(DecodeInfo *decInfo)
{
    char buffer[32];
    if (fread(buffer, 1, 32, decInfo->fptr_stego_image) != 32)
        return e_failure;
    decInfo->size_secret_file = decode_size_from_lsb(buffer);
    return e_success;
}

// Function: decode_secret_file_data
// Purpose : Decode actual file data (byte by byte)
Status decode_secret_file_data(DecodeInfo *decInfo)
{
        if(!decInfo||!decInfo->fptr_stego_image||!decInfo->fptr_output)
                return e_failure;
                
        char *data=malloc(decInfo->size_secret_file);
        if(!data)
        return e_failure;

        if(decode_data_from_image(data,decInfo->size_secret_file,decInfo->fptr_stego_image)==e_failure)
        {
                free(data);
                return e_failure;
        }

        if(fwrite(data,1,decInfo->size_secret_file,decInfo->fptr_output)!=decInfo->size_secret_file)
        {
                free(data);
                return e_failure;
        }

        free(data);
        return e_success;
}

// Function: decode_data_from_image
// Purpose : Decode 'size' number of bytes from image (each byte from 8 pixels)
Status decode_data_from_image(char *data,int size,FILE *fptr_stego_image)
{
        if(!data||size<=0||!fptr_stego_image)
                return e_failure;

        char image_buffer[8];

        for(int i=0;i<size;i++)
        {
                if(fread(image_buffer,1,8,fptr_stego_image)!=8)
                {
                        fprintf(stderr,"ERROR:Unexpected EOF\n");
                        //error message if file encounters EOF before expected
                        return e_failure;
                }
                data[i]=decode_byte_from_lsb(image_buffer);
        }

        return e_success;
}

// Function: decode_byte_from_lsb
// Purpose : Extract 1 byte from 8 LSBs of image data
char decode_byte_from_lsb(char *image_buffer)
{
        char data=0;
        for(int i=0;i<8;i++)//for 8 bits decoding 
        {
                data=(data<<1)|(image_buffer[i]&1);
        }
        return data;
}

// Function: decode_size_from_lsb
// Purpose : Extract integer size (32 bits) from image data
int decode_size_from_lsb(const char *image_buffer)
{
    int size = 0;
    for (int i = 0; i < 32; i++) // 32 bits for int
    {
        //combine all lsb bits
        size =(size<<1)|(image_buffer[i]&1);
    }
    return size;
}
