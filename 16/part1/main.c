#define _GNU_SOURCE
#include <ctype.h>
#include <errno.h>
#include <error.h>
#include <fcntl.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

const int phase_pattern[4] = {0, 1, 0, -1};
const int phase_len = 4;

int abs( int i ) {
    if ( i < 0 )
        return -i;
    return i;
}

void computePhase( char **input_in, const size_t input_len,
                   const int *phase_pattern, const int phase_len, int count, size_t offset ) {
    int mulindex = 0;
    int addmul = 1;
    char *input = *input_in;
    int num = 0;
    for( size_t i = offset; i < input_len; i++ ) {
        input[i] -= '0';
    }
    if( offset > input_len/2 ) {
        for ( int a = 0; a < count; a++ ) {
            for ( size_t i = input_len - 2; i >= offset; i-- ) {
                input[i] += input[i+1];
                input[i] %= 10;
            }
        }
    } else {
        for ( int a = 0; a < count; a++ ) {
            addmul = offset + 1;

            for ( size_t i = offset; i < input_len; i++ ) {
                mulindex = 0;
                num = 0;
                for ( size_t j = i; j < input_len; j += addmul ) {
                    mulindex++;
                    if ( mulindex >= phase_len )
                        mulindex = 0;
                    if( phase_pattern[mulindex] == -1 ) {
                        for( int k = 0; k < addmul; k++ ) {
                            if( j + k >= input_len )
                                break;
                            num -= input[j + k];
                        }
                    } else if ( phase_pattern[mulindex] == 1 ) {
                        for( int k = 0; k < addmul; k++ ) {
                            if( j + k >= input_len )
                                break;
                            num += input[j + k];
                        }
                    }
                }
                input[i] = abs( num ) % 10;
                addmul++;
            }
        }
    }
    for( size_t i = offset; i < input_len; i++ ) {
        input[i] += '0';
    }
}

int main() {
    FILE *in = fopen( "input", "r" );
    char *input_og = NULL;
    size_t input_buf = 0;
    size_t input_len = getline( &input_og, &input_buf, in );
    input_og[input_len - 1] = '\0';
    input_len--;
    char *input = strdup( input_og );

//--[ Part 1 ]-----------------------------------------------------------------
    computePhase( &input, input_len, phase_pattern, phase_len, 100, 0 );
    input[8] = '\0';
    printf( "FIRST 8 DIGITS OF THE FINAL OUTPUT ARE: %s\n", input );
//--[ Part 2 ]-----------------------------------------------------------------
    char *input_long = malloc( 10000 * input_len );
    input_len *= 10000;
    input_long[0] = '\0';
    for( int i = 0; i < 10000; i++ ) {
        strcat( input_long, input_og );
    }
    input_og[7] = '\0';
    size_t offset = strtoul( input_og, NULL, 10 );
    computePhase( &input_long, input_len, phase_pattern, phase_len, 100, offset );
    input_long[offset + 8] = '\0';
    printf( "REQUESTED 8 DIGITS ARE: %s\n", input_long + offset );

    free( input );
    free( input_og );
//    free( input_long );
    fclose( in );
}
