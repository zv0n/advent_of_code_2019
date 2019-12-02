#define _GNU_SOURCE
#include <errno.h>
#include <error.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

size_t populateCode( int **code, char *input ) {
    char *end = input;
    size_t array_len = 1024;
    size_t code_len = 0;
    *code = malloc( array_len * sizeof( int ) );
    if ( code == NULL )
        error( EXIT_FAILURE, errno, "malloc" );
    while ( *end != '\0' && *end != '\n' ) {
        ( *code )[code_len] = strtoul( input, &end, 10 );
        input = end + 1;
        code_len++;
        if ( code_len == array_len ) {
            array_len *= 2;
            int *tmp = realloc( *code, array_len );
            if ( tmp == NULL )
                error( EXIT_FAILURE, errno, "realloc" );
            *code = tmp;
        }
    }
    return code_len;
}

void compute( int *code, size_t code_len ) {
    int res = 0;
    for ( size_t i = 0; i < code_len; i += 4 ) {
        switch ( code[i] ) {
        case 1:
            if ( i + 3 >= code_len || (size_t)code[i + 3] >= code_len )
                error( EXIT_FAILURE, 0,
                       "INSTRUCTION POINTS BEYOND ARRAY, INDEX: %zu\n", i );
            res = code[code[i + 1]] + code[code[i + 2]];
            code[code[i + 3]] = res;
            break;
        case 2:
            if ( i + 3 >= code_len || (size_t)code[i + 3] >= code_len )
                error( EXIT_FAILURE, 0,
                       "INSTRUCTION POINTS BEYOND ARRAY, INDEX: %zu\n", i );
            res = code[code[i + 1]] * code[code[i + 2]];
            code[code[i + 3]] = res;
            break;
        case 99:
            return;
        default:
            error( EXIT_FAILURE, 0,
                   "INCORRECT INSTRUCTION AT INDEX: %zu, INSTRUCTION IS: %i\n",
                   i, code[i] );
        }
    }
}

int main() {
    FILE *in = fopen( "input", "r" );
    char *input = NULL;
    size_t input_len = 0;
    getline( &input, &input_len, in );
    int *code = NULL;
    size_t code_len = populateCode( &code, input );
    int *working_code = malloc( code_len * sizeof( int ) );
    if( working_code == NULL )
        error( EXIT_FAILURE, errno, "malloc" );
    bool stop_looping = false;
    for( int i = 0; i < 100; i++ ) {
        for( int j = 0; j < 100; j++ ) {
            memcpy( working_code, code, code_len * sizeof( int ) );
            working_code[1] = i;
            working_code[2] = j;
            compute( working_code, code_len );
            if( working_code[0] == 19690720 ) {
                stop_looping = true;
                printf( "%i\n", 100 * i + j );
                break;
            }
        }
        if( stop_looping )
            break;
    }
    free( code );
    free( input );
}
