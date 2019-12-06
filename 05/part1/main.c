#define _GNU_SOURCE
#include <errno.h>
#include <error.h>
#include <fcntl.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

#define PARAM_0 0x0001
#define PARAM_1 0x0002
#define PARAM_2 0x0004
// future proofing
#define PARAM_3 0x0008
#define PARAM_4 0x0010
#define PARAM_5 0x0020
#define PARAM_6 0x0040
#define PARAM_7 0x0080

const int param_flags[] = {PARAM_0, PARAM_1, PARAM_2, PARAM_3,
                           PARAM_4, PARAM_5, PARAM_6, PARAM_7};

size_t populateCode( int **code, char *input ) {
    char *end = input;
    size_t array_len = 1024;
    size_t code_len = 0;
    *code = malloc( array_len * sizeof( int ) );
    if ( code == NULL )
        error( EXIT_FAILURE, errno, "malloc" );
    while ( *end != '\0' && *end != '\n' ) {
        ( *code )[code_len] = strtoimax( input, &end, 10 );
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

void testOutOfBounds( size_t code_len, size_t position ) {
    if ( position >= code_len )
        error( EXIT_FAILURE, 0, "INSTRUCTION POINTS BEYOND ARRAY, INDEX: %zu\n",
               position );
}

int getInstruction( int code, size_t *flags ) {
    int ret = code % 100;
    code /= 100;
    int i = 0;
    while ( code > 0 ) {
        if ( code % 10 == 1 )
            *flags |= param_flags[i];
        code /= 10;
        i++;
    }
    return ret;
}

void compute( int *code, size_t code_len ) {
    int res = 0;
    int temp_param = 0;
    char *end = NULL;
    char *input = NULL;
    size_t input_len = 0;
    size_t flags = 0;
    int instruction = 0;
    for ( size_t i = 0; i < code_len; ) {
        instruction = getInstruction( code[i], &flags );
        switch ( instruction ) {
        case 1:
        case 2:
            testOutOfBounds( code_len, i + 3 );
            if ( flags & PARAM_0 ) {
                res = code[i + 1];
            } else {
                testOutOfBounds( code_len, code[i + 1] );
                res = code[code[i + 1]];
            }
            if ( flags & PARAM_1 ) {
                temp_param = code[i + 2];
            } else {
                testOutOfBounds( code_len, code[i + 2] );
                temp_param = code[code[i + 2]];
            }
            if( instruction == 1 )
                res += temp_param;
            else
                res *= temp_param;
            if ( flags & PARAM_2 ) {
                error( EXIT_FAILURE, 0,
                       "LAST PARAM MUST BE IN MODE 0! Index: %zu", i );
            } else {
                testOutOfBounds( code_len, code[i + 3] );
                code[code[i + 3]] = res;
            }
            i += 4;
            break;
        case 3:
            testOutOfBounds( code_len, i + 1 );
            printf( "INPUT: " );
            if ( flags & PARAM_0 ) {
                error( EXIT_FAILURE, 0,
                       "LAST PARAM MUST BE IN MODE 0! Index: %zu", i );
            } else {
                testOutOfBounds( code_len, code[i + 1] );
                getline( &input, &input_len, stdin );
                res = strtoimax( input, &end, 10 );
                if ( *end != '\n' && *end != '\0' )
                    error( EXIT_FAILURE, 0,
                           "GOT INPUT THAT'S NOT AN INTEGER: '%s'\n", input );
                code[code[i + 1]] = res;
            }
            i += 2;
            break;
        case 4:
            testOutOfBounds( code_len, i + 1 );
            if ( flags & PARAM_0 ) {
                testOutOfBounds( code_len, i + 1 );
                printf( "%i\n", code[i + 1] );
            } else {
                testOutOfBounds( code_len, code[i + 1] );
                printf( "%i\n", code[code[i + 1]] );
            }
            i += 2;
            break;
        case 99:
            return;
        default:
            error( EXIT_FAILURE, 0,
                   "INCORRECT INSTRUCTION AT INDEX: %zu, INSTRUCTION IS: %i\n",
                   i, code[i] );
        }
        flags = 0;
    }
}

int main() {
    FILE *in = fopen( "input", "r" );
    char *input = NULL;
    size_t input_len = 0;
    getline( &input, &input_len, in );
    int *code = NULL;
    size_t code_len = populateCode( &code, input );
    compute( code, code_len );
    free( code );
    free( input );
}
