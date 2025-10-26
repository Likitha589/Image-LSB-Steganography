#ifndef DECODE_H
#define DECODE_H

#include "types.h"

typedef struct _DecodeInfo
{
    /* Stego Image Info */
    char *stego_image_fname;
    FILE *fptr_stego_image;

    /* Output File Info */
    char *output_fname;
    FILE *fptr_output;

    /* Decoded Data */
    int extn_size;
    char extn_secret_file[10];//char array not a string literal
    long size_secret_file;
}DecodeInfo;

//Function Prototypes

Status read_and_validate_decode_args(int argc, char *argv[], DecodeInfo *decInfo);

Status open_decode_files(DecodeInfo *decInfo);

Status do_decoding(DecodeInfo *decInfo);

Status decode_magic_string(DecodeInfo *decInfo);

Status decode_secret_file_extn_size(DecodeInfo *decInfo);

Status decode_secret_file_extn(DecodeInfo *decInfo);

Status decode_secret_file_size(DecodeInfo *decInfo);

Status decode_secret_file_data(DecodeInfo *decInfo);

Status decode_data_from_image(char *output, int size, FILE *fptr_stego_image);

char decode_byte_from_lsb(char *image_buffer);

int decode_size_from_lsb(const char *image_buffer);
#endif