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
                   const int *phase_pattern, const int phase_len, int count ) {
    int mulindex = 0;
    int addmul = 1;
    char *input = *input_in;
    int num = 0;
    for ( int a = 0; a < count; a++ ) {
        addmul = 1;
        char *output = malloc( input_len + 1 );
        output[input_len] = '\0';

        for ( size_t i = 0; i < input_len; i++ ) {
            mulindex = 0;
            num = 0;
            for ( size_t j = 0; j < input_len; j++ ) {
                if ( ( j + 1 ) % addmul == 0 )
                    mulindex++;
                if ( mulindex >= phase_len )
                    mulindex = 0;
                num += ( input[j] - '0' ) * phase_pattern[mulindex];
            }
            output[i] = abs( num % 10 ) + '0';
            addmul++;
        }
        *input_in = output;
        free( input );
        input = output;
    }
}

int main() {
    FILE *in = fopen( "input", "r" );
    char *input = NULL;
    size_t input_buf = 0;
    size_t input_len = getline( &input, &input_buf, in );
    input[input_len - 1] = '\0';
    input_len--;

    computePhase( &input, input_len, phase_pattern, phase_len, 100 );
    input[8] = '\0';
    printf( "FIRST 8 DIGITS OF THE FINAL OUTPUT ARE: %s\n", input );

    free( input );
    fclose( in );
}
