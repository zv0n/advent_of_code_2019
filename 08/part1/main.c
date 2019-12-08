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

#define PIC_WIDTH 25
#define PIC_HEIGHT 6

void countOccurences( char *input, int *occurences, size_t layer_len ) {
    occurences[0] = occurences[1] = occurences[2] = 0;
    for( size_t i = 0; i < layer_len; i++ ) {
        switch( input[i] - '0' ) {
            case 0:
                occurences[0] += 1;
                break;
            case 1:
                occurences[1] += 1;
                break;
            case 2:
                occurences[2] += 1;
                break;
            default:
                error( EXIT_FAILURE, 0, "UNEXPECTED INPUT! %c", input[i] );
        }
    }
}

int main() {
    FILE *in = fopen( "input", "r" );

    int min[3] = {INT_MAX, 0, 0};
    int tmp[3] = {0, 0, 0};

    char *input = NULL;
    size_t alloc_len = 0;
    size_t input_len = 0;
    if ( ( input_len = getline( &input, &alloc_len, in ) ) <= 0 )
        error( EXIT_FAILURE, errno, "getline" );
    size_t og_input_len = input_len;
    const int layer_len = PIC_WIDTH * PIC_HEIGHT;
    while ( input_len >= layer_len ) {
        countOccurences( input + ( og_input_len - input_len ), tmp, layer_len );
        input_len -= layer_len;
        if ( tmp[0] < min[0] )
            memcpy( min, tmp, 3 * sizeof( int ) );
    }
    printf( "LAYER WITH LEAST 0s HAS: %i ZEROS, %i ONES, %i TWOS\nPUZZLE "
            "SOLUTION: %i\n",
            min[0], min[1], min[2], min[1] * min[2] );
    free( input );
}
