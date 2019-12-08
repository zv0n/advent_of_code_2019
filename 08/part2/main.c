#define _GNU_SOURCE
#include <ctype.h>
#include <errno.h>
#include <error.h>
#include <fcntl.h>
#include <inttypes.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "bitmap.h"

#define PIC_WIDTH 25
#define PIC_HEIGHT 6

void getPixels( char *input, int *pixels, size_t layer_len ) {
    for ( size_t i = 0; i < layer_len; i++ ) {
        if ( pixels[i] == 2 && ( input[i] - '0' ) != 2 ) {
            pixels[i] = input[i] - '0';
        }
    }
}

void flipPixels( int *output, int *input ) {
    for( int i = 0; i < PIC_HEIGHT; i++ ) {
        for( int j = 0; j < PIC_WIDTH; j++ ) {
            output[(PIC_HEIGHT-i-1)*PIC_WIDTH + j] = input[i*PIC_WIDTH + j];
        }
    }
}

int main() {
    FILE *in = fopen( "input", "r" );

    char *input = NULL;
    size_t alloc_len = 0;
    size_t input_len = 0;
    if ( ( input_len = getline( &input, &alloc_len, in ) ) <= 0 )
        error( EXIT_FAILURE, errno, "getline" );
    size_t og_input_len = input_len;
    const size_t layer_len = PIC_WIDTH * PIC_HEIGHT;
    int pixels[layer_len];
    for ( size_t i = 0; i < layer_len; i++ ) {
        pixels[i] = 2;
    }
    while ( input_len >= layer_len ) {
        getPixels( input + ( og_input_len - input_len ), pixels, layer_len );
        input_len -= layer_len;
    }

    int pixels_out[layer_len];
    flipPixels( pixels_out, pixels );

    writeBitmap( pixels_out, PIC_WIDTH, PIC_HEIGHT );
    printf( "ANSWER IS IN FILE OUTPUT.BMP\n" );
    free( input );
}
