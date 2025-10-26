#include <stdio.h>
#include <string.h>
#include "encode.h"
#include "types.h"
#include "common.h"

#define RESET   "\033[0m"
#define RED     "\033[1;31m"
#define GREEN   "\033[1;32m"
#define MAGENTA "\033[1;35m"
#define WHITE   "\033[1;37m"

/* Function Definitions */

//Step 1: Check argv[2] is .bmp or not, if yes goto step 2, if not goto step 3(We can check signature 2bytes BM too)
//Step 2: Store into src_image_fname(encInfo->src_image_fname=argv[2])
//Step 3: return e_failure

//Step 4: check argv[3] is .txt file or not, if yes goto step 5,if not goto step 6
//Step 5: i) store the filename into secret_fname
        //ii) fetch the extension and store into the extn_sec-file(strcpy)
//Step 6: return e_failute

//Step 7: check argv[4] is !=NULL,if yes goto step 8 , if no go to step 12
//Step 8: if yes, goto step 9

//Step 9: check  the file is .bmp or not, if yes goto step 10, if not goto step 11
//Step 10: store the filename into the stego_image_fname , return e_success
//Step 11: return e_failure

//Step 12: store the default filename[stego.bmp] into the stego_image_fname
//return e_success
Status read_and_validate_encode_args(int argc,char *argv[], EncodeInfo *encInfo)
{
    if(argc<3||argv[2]==NULL||strstr(argv[2],".bmp")==NULL)
    {        
        fprintf(stderr,RED"ERROR: Source image file is not provided\n"RESET);
        return e_failure;
    }
    encInfo->src_image_fname=argv[2];

    if(argc<4||argv[3]==NULL)
    {
        fprintf(stderr,RED"ERROR: Secret file is required!\n"RESET);
        return e_failure;
    }
    else if(strstr(argv[3],".bmp"))
    {
        printf(RED"ERROR: Secret file shouldn't be a bmp file.\n"RESET);
        return e_failure;
    }

    encInfo->secret_fname=argv[3];

    char *extn=strstr(argv[3],".");
    if(extn!=NULL)
    {
        strncpy(encInfo->extn_secret_file, extn, MAX_FILE_SUFFIX);//MAX_FILE_SUFFIX=4
        encInfo->extn_secret_file[MAX_FILE_SUFFIX] = '\0';
    }
    else
    {
            encInfo->extn_secret_file[0]='\0';
    }

    if(argc>=5 && argv[4]!=NULL)
    {
        if(strstr(argv[4],".bmp")==NULL)
        {
            printf(RED "ERROR: Please provide .bmp extension to optional file\n");
            return e_failure;
        }
        else if(strstr(argv[4],".bmp")!=NULL)
        encInfo->stego_image_fname=argv[4];
    }
    else
    {
        printf(WHITE"INFO: Output file not mentioned. Creating <stego.bmp> as default\n"RESET);
        encInfo->stego_image_fname="stego.bmp";
    }

    return e_success;
}

/* Get image size
 * Input: Image file ptr
 * Output: width * height * bytes per pixel (3 in our case)
 * Description: In BMP Image, width is stored in offset 18,
 * and height after that. size is 4 bytes
 */
long get_image_size_for_bmp(FILE *fptr_image)
{
    uint width=0, height=0;
    // Seek to 18th byte
    if(fseek(fptr_image, 18, SEEK_SET)!=0)
            return 0;
    // Read the width (an int)
    if(fread(&width, sizeof(int), 1, fptr_image)!=1)
            return 0;
    //printf("width = %u\n", width);
8
    // Read the height (an int)
    if(fread(&height, sizeof(int), 1, fptr_image)!=1)
            return 0;
    //printf("height = %u\n", height);

    // Return image capacity
    return width * height * 3;
}

//Step 1: move the file pointer to the end(fseek)
//Step 2: return by using ftell
long get_file_size(FILE *fptr)
{
    // 2. Get secret file size
    if(fptr==NULL)
            return 0;
    if(fseek(fptr, 0, SEEK_END)!=0)
    //fseek return type is int 
    //returns non-zero on failure and 0 on success
            return 0;
    long size=ftell(fptr);
    if(size<0)
            return 0;

    rewind(fptr);//can rewind here
    printf(GREEN"INFO: Done!Not Empty\n"RESET);
    return (uint)size;
    //fseek(fptr, 0, SEEK_SET); or rewind
}

/* Perform encoding */

Status do_encoding(EncodeInfo *encInfo)
{
    printf(MAGENTA"INFO: ## Encoding Procedure Started ##\n"RESET);

    printf(WHITE "INFO: Opening required files...!\n" RESET);
    if(open_files(encInfo)==e_failure)//src img and secret file on 'rb' but stego image on 'wb'
    {
        //print the error msg
        //return e_failure
        fprintf(stderr,RED "ERROR: Opening files failed\n" RESET);
        return e_failure;
    }

    //print the success message
    printf(GREEN "INFO: Files opened Successfully\n" RESET);

    // 1. Get image size
    encInfo->image_capacity = get_image_size_for_bmp(encInfo->fptr_src_image);

    printf(WHITE "INFO: Checking image capacity...\n" RESET);
    // 3. Check capacity
    if (check_capacity(encInfo) == e_failure)
    {
        fprintf(stderr, RED"ERROR: Image does not have enough capacity\n"RESET);
        return e_failure;
    }
    //print success msg
    printf(GREEN "INFO: Image has enough capacity\n"RESET);

    // 4. Copy BMP header (first 54 bytes) to output
    printf(WHITE "INFO: Copying BMP Header...\n" RESET);
    if (copy_bmp_header(encInfo->fptr_src_image, encInfo->fptr_stego_image) == e_failure)
    {
        //print error msg
        fprintf(stderr,RED"ERROR: Failed to copy BMP Header\n"RESET);
        return e_failure;
    }
    //print success msg
    printf(GREEN "INFO: BMP Header Copied Successfully\n"RESET);

    // 5. Encode Magic String
       //Status encode_data_to_image(char *data, int size, FILE *fptr_src_image, FILE *fptr_stego_image)
      //acts as a generic function
     //get clear set
    //dont call byte_to_lsb here
    printf(WHITE "INFO: Encoding magic string...\n" RESET);
    if (encode_magic_string(MAGIC_STRING, encInfo) == e_failure)
    {
        //print the error message
        fprintf(stderr,RED"ERROR: Failed to encode secret file size\n"RESET);
        return e_failure;
    }
    //print the success msg
    printf(GREEN"INFO: Magic String encoded successfully\n"RESET);

    //6.Encode the extension size
    printf(WHITE "INFO: Encoding file extension size...\n" RESET);
    if(encode_secret_file_extn_size(strlen(encInfo->extn_secret_file),encInfo)==e_failure)
    {
            //print the error msg
            fprintf(stderr,RED"ERROR: Failed to encode extension size\n"RESET);
            return e_failure;
    }
    //print the success msg
    printf(GREEN"INFO: Extension size encoded successfully\n");
    //7.Encode the secret file extension
    printf(WHITE "INFO: Encoding file extension...\n" RESET);
    if(encode_secret_file_extn(encInfo->extn_secret_file,encInfo)==e_failure)
    {
            //print error msg
            fprintf(stderr,RED"ERROR: Failed to encode extension data\n"RESET);
            return e_failure;
    }
    //print success msg
    printf(GREEN "INFO: Extension encoded successfully\n" RESET);
    // 7. Encode Secret File Size
    printf(WHITE "INFO: Encoding secret file size...\n"RESET);
    if (encode_secret_file_size(encInfo->size_secret_file, encInfo) == e_failure)
    {
        fprintf(stderr,RED "ERROR : Failed to encode secret file size\n"RESET);
        return e_failure;
    }
    printf(GREEN "INFO: Secret file size encoded successfully\n" RESET);

    // 7. Encode Secret File Data
    printf(WHITE "INFO: Encoding secret file data...\n"RESET);
    if (encode_secret_file_data(encInfo) == e_failure)
    {
        fprintf(stderr,RED"ERROR:Failed to encode secret file data\n"RESET);
        return e_failure;
    }
    printf(GREEN"INFO: Secret file data encoded successfully\n"RESET);

    // 8. Copy remaining image data
    printf(WHITE "INFO: Copying remaining image data...\n"RESET);
    if (copy_remaining_img_data(encInfo->fptr_src_image, encInfo->fptr_stego_image) == e_failure)
    {
        fprintf(stderr,RED"ERROR: Failed to copy remaining image data\n"RESET);
        return e_failure;
    }
    printf(GREEN"INFO: Remaining image data copied successfully\n"RESET);
    
    //printf(GREEN "INFO: ## Encoding Completed Successfully! ##\n"RESET);
    return e_success;
}


Status check_capacity(EncodeInfo *encInfo)
{
    unsigned long int s1=get_image_size_for_bmp(encInfo->fptr_src_image);
    encInfo->size_secret_file=get_file_size(encInfo->fptr_secret);
    //to encode size we can directly hard code 32 because size is always int(4 bytes)
    if(s1 > 54 + (int)strlen(MAGIC_STRING)*8 + 32 + (int)strlen(encInfo->extn_secret_file)*8 + 32 +((encInfo->size_secret_file)*8))
    {
        printf(GREEN"INFO: Done!\n"RESET);
        return e_success;
    }
    else
    return e_failure;
}

   //rewind the source file pointer
  //Step 1: char image_buffer[54]; Read 54 bytes from the source
 //Step 2: write those 54 bytes into the destination
//return e_success
Status copy_bmp_header(FILE *fptr_src_image, FILE *fptr_dest_image)
{
    char header[54];
    rewind(fptr_src_image);

    if (fread(header, 1, 54, fptr_src_image) != 54)
    {
        fprintf(stderr, RED"ERROR: Failed to read BMP header\n"RESET);
        return e_failure;
    }

    fwrite(header, 1, 54, fptr_dest_image);
    return e_success;
}

Status encode_data_to_image(const char *data, int size, FILE *fptr_src_image, FILE *fptr_stego_image)
{
      if (!data || size <= 0 || !fptr_src_image || !fptr_stego_image)
        return e_failure;
      //declare image buffer with size 8 bytes
      char image_buffer[8];
      for(int i=0;i<size;i++)
      {
         //Step 1: read 8 bytes from the source
        //Step 2: call the encode_byte_to_lsb(char data, char *image_buffer); function
       //encode_byte_to_lsb(data[i], image_buffer);-> 1 character '#'
      //Step 3: fwrite the encoded data(8 bytes) into stego_image
      fread(image_buffer,1,8,fptr_src_image);
      encode_byte_to_lsb(data[i],image_buffer);
      fwrite(image_buffer,1,8,fptr_stego_image);
      }
      return e_success;

}

//encode magic string
Status encode_magic_string(const char *magic_string, EncodeInfo *encInfo)
{
    if (!magic_string || !encInfo)
        return e_failure;
    //generic function we can call this function in every encoding function
    return encode_data_to_image((char*)magic_string, strlen(magic_string), encInfo->fptr_src_image, encInfo->fptr_stego_image);
    printf("Done\n");
}

//extension file size and extension encoding has to be done
Status encode_secret_file_extn_size(long extn_size, EncodeInfo *encInfo)
{
        //call byte_to_size
        char image_buffer[32];
        fread(image_buffer,1,32,encInfo->fptr_src_image);
        encode_size_to_lsb(extn_size,image_buffer);
        fwrite(image_buffer,1,32,encInfo->fptr_stego_image);
        return e_success;
}

Status encode_secret_file_extn(const char *extn,EncodeInfo *encInfo)
{
        if(!extn || !encInfo)
        return e_failure;
        //call encode_data_from_image() generic function
        return encode_data_to_image(extn,strlen(extn),encInfo->fptr_src_image,encInfo->fptr_stego_image);
}

//encode secret file size
Status encode_secret_file_size(long file_size,EncodeInfo *encInfo)
{
        if (!encInfo || !encInfo->fptr_src_image || !encInfo->fptr_stego_image)
        return e_failure;
        char image_buffer[32];
        fread(image_buffer,1,32,encInfo->fptr_src_image);
           //call encode_size_to_lsb(encInfo->size_secret_file,encInfo)
          //step 1: read 32 bytes from the src
         //step 2; call a size_to_lsb(file_size,image_buffer)
        //step 3: write 32 bytes to the dest
        encode_size_to_lsb(file_size,image_buffer);
        fwrite(image_buffer,1,32,encInfo->fptr_stego_image);
        return e_success;
}

//encode secret file data
Status encode_secret_file_data(EncodeInfo *encInfo)
{
        if (!encInfo || !encInfo->fptr_secret || !encInfo->fptr_src_image || !encInfo->fptr_stego_image)
        return e_failure;
        char ch;
           //char image_buffer[encInfo->size_secret_file];(or) char image_buffer[8];
          //rewind
         //read
        //encode_data_to_image(data,encInfo->size_secret_file,encInfo->fptr_src_image,encInfo->fptr_stego_image);
        while(fread(&ch,1,1,encInfo->fptr_secret))//read char by char till EOF
        {
                char image_buffer[8];
                fread(image_buffer,1,8,encInfo->fptr_src_image);//read 1 char to encode bit by bit
                encode_byte_to_lsb(ch,image_buffer);
                fwrite(image_buffer,1,8,encInfo->fptr_stego_image);//write those char into dest image 
        }
        return e_success;
}

//copy remaining data after encoding
Status copy_remaining_img_data(FILE *fptr_src_image,FILE *fptr_dest_image)
{
        if (!fptr_src_image || !fptr_dest_image)
        return e_failure;
        char ch;
        while(fread(&ch,1,1,fptr_src_image))//read
        {
                fwrite(&ch,1,1,fptr_dest_image);//write
        }
        return e_success;
}


Status encode_byte_to_lsb(char data, char *image_buffer)
{
          //Step 1: get msb bit in data
         //Step 2: clear the lsb bit in image buffer of i
        //Step 3: step 1|step 2
    for (int i = 0; i < 8; i++)
    {
        int bit = (data >> (7-i))&1;
        image_buffer[i] = (image_buffer[i]& ~1) | bit;
    }
    return e_success;
}

Status encode_size_to_lsb(int data,char *image_buffer)
{
          //Step 1: get the msb bit in data
         //Step 2: clear the lsb bit in image buffer
        //Step 3: replace the lsb bit
    for (int i = 0; i < 32; i++)
    {
        int bit = (data >> (31-i))&1;
        image_buffer[i] = (image_buffer[i]& ~1) | bit;
    }
    return e_success;

}
/*
 * Get File pointers for i/p and o/p files
 * Inputs: Src Image file, Secret file and
 * Stego Image file
 * Output: FILE pointer for above files
 * Return Value: e_success or e_failure, on file errors
 */
Status open_files(EncodeInfo *encInfo)
{
    // Src Image file
    encInfo->fptr_src_image = fopen(encInfo->src_image_fname, "rb");//read mode
    // Do Error handling
    if (encInfo->fptr_src_image == NULL)
    {
        perror(RED"ERROR: fopen"RESET);
        fprintf(stderr,RED "ERROR: Unable to open file %s\n"RESET, encInfo->src_image_fname);

        return e_failure;
    }
    printf(WHITE"INFO: Opened %s\n"RESET,encInfo->src_image_fname);

    // Secret file
    encInfo->fptr_secret = fopen(encInfo->secret_fname, "rb");//read mode
    // Do Error handling
    if (encInfo->fptr_secret == NULL)
    {
        perror(RED"ERROR: fopen"RESET);
        fprintf(stderr, RED"ERROR: Unable to open file %s\n"RESET, encInfo->secret_fname);

        return e_failure;
    }
    printf(WHITE"INFO: Opened %s\n"RESET,encInfo->secret_fname);

    // Stego Image file
    encInfo->fptr_stego_image = fopen(encInfo->stego_image_fname, "wb");//write mode
    // Do Error handling
    if (encInfo->fptr_stego_image == NULL)
    {
        perror(RED"ERROR: fopen"RESET);
        fprintf(stderr, RED"ERROR: Unable to open file %s\n"RESET, encInfo->stego_image_fname);

        return e_failure;
    }
    printf(WHITE"INFO: Opened %s\n"RESET,encInfo->stego_image_fname);
    printf(GREEN"INFO: Done\n"RESET);
    // No failure return e_success
    return e_success;
}                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                          