#define _GNU_SOURCE
#include <errno.h>
#include <error.h>
#include <fcntl.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

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
            if ( instruction == 1 )
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
            //            printf( "INPUT: " );
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
                printf( "%i\n", code[i + 1] );
            } else {
                testOutOfBounds( code_len, code[i + 1] );
                printf( "%i\n", code[code[i + 1]] );
            }
            i += 2;
            break;
        case 5:
        case 6:
            testOutOfBounds( code_len, i + 2 );
            if ( flags & PARAM_0 ) {
                temp_param = code[i + 1];
            } else {
                testOutOfBounds( code_len, code[i + 1] );
                temp_param = code[code[i + 1]];
            }
            if ( ( instruction == 5 && temp_param != 0 ) ||
                 ( instruction == 6 && temp_param == 0 ) ) {
                if ( flags & PARAM_1 ) {
                    i = code[i + 2];
                } else {
                    testOutOfBounds( code_len, code[i + 2] );
                    i = code[code[i + 2]];
                }
            } else {
                i += 3;
            }
            break;
        case 7:
        case 8:
            testOutOfBounds( code_len, i + 2 );
            if ( flags & PARAM_0 ) {
                res = code[i + 1];
            } else {
                testOutOfBounds( code_len, code[i + 1] );
                res = code[code[i + 1]];
            }
            if ( flags & PARAM_1 ) {
                if ( instruction == 7 )
                    res = res < code[i + 2] ? 1 : 0;
                else
                    res = res == code[i + 2] ? 1 : 0;
            } else {
                testOutOfBounds( code_len, code[i + 2] );
                if ( instruction == 7 )
                    res = res < code[code[i + 2]] ? 1 : 0;
                else
                    res = res == code[code[i + 2]] ? 1 : 0;
            }
            if ( flags & PARAM_2 ) {
                error( EXIT_FAILURE, 0,
                       "LAST PARAM MUST BE IN MODE 0! Index: %zu", i );
            } else {
                testOutOfBounds( code_len, code[i + 3] );
                code[code[i + 3]] = res;
            }
            i += 4;
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

bool permutation( int *phase ) {
    bool permutation = true;
    for ( int i = 0; i < 5; i++ ) {
        for ( int j = 0; j < 5; j++ ) {
            if ( phase[i] == phase[j] && i != j ) {
                permutation = false;
                break;
            }
        }
        if ( !permutation )
            break;
    }
    return permutation;
}

void increasePhase( int *phase ) {
    do {
        for ( int i = 0; i < 5; i++ ) {
            phase[i] += 1;
            if ( phase[i] == 5 )
                phase[i] = 0;
            else
                break;
        }
    } while ( !permutation( phase ) );
}

int main() {
    //--[ Initialize computer
    //]----------------------------------------------------
    FILE *in = fopen( "input", "r" );
    char *input = NULL;
    size_t input_len = 0;
    getline( &input, &input_len, in );
    int *code = NULL;
    size_t code_len = populateCode( &code, input );
    //--[ Compute thrusters
    //]------------------------------------------------------
    int pipes_in[2];
    if ( pipe( pipes_in ) == -1 )
        error( EXIT_FAILURE, errno, "pipe" );
    int pipes_out[2];
    if ( pipe( pipes_out ) == -1 )
        error( EXIT_FAILURE, errno, "pipe" );
    int phase[5] = {0, 1, 2, 3, 4};
    size_t maxphase = 0;
    size_t maxthrust = 0;
    int *tmp = malloc( code_len );
    if ( tmp == NULL )
        error( EXIT_FAILURE, errno, "malloc" );
    memcpy( tmp, code, code_len );
    const int *code_backup = tmp;
    char phase_input[2];
    size_t pipe_read = 0;
    FILE *output = fdopen( pipes_in[0], "r" );
    char *pipe_input = NULL;
    size_t pipe_size = 0;
    if ( output == NULL )
        error( EXIT_FAILURE, errno, "fdopen" );
    printf( "STARTING COMPUTING!\n" );
    for ( int i = 0; i < 120; i++ ) {
        int pid = fork();
        if ( pid > 0 ) {
            if ( dup2( pipes_in[1], STDOUT_FILENO ) == -1 )
                error( EXIT_FAILURE, errno, "dup2" );
            if ( dup2( pipes_out[0], STDIN_FILENO ) == -1 )
                error( EXIT_FAILURE, errno, "dup2" );
            close( pipes_in[0] );
            close( pipes_out[1] );
            compute( code, code_len );
            memcpy( code, code_backup, code_len );
            compute( code, code_len );
            memcpy( code, code_backup, code_len );
            compute( code, code_len );
            memcpy( code, code_backup, code_len );
            compute( code, code_len );
            memcpy( code, code_backup, code_len );
            compute( code, code_len );
            exit( 0 );
        } else if ( pid < 0 ) {
            error( EXIT_FAILURE, errno, "fork" );
        }
        snprintf( phase_input, 2, "%i\n", phase[0] );
        phase_input[1] = '\n';
        write( pipes_out[1], phase_input, 2 );
        write( pipes_out[1], "0\n", 2 );
        pipe_read = getline( &pipe_input, &pipe_size, output );
        snprintf( phase_input, 2, "%i\n", phase[1] );
        phase_input[1] = '\n';
        write( pipes_out[1], phase_input, 2 );
        write( pipes_out[1], pipe_input, pipe_read );
        pipe_read = getline( &pipe_input, &pipe_size, output );
        snprintf( phase_input, 2, "%i\n", phase[2] );
        phase_input[1] = '\n';
        write( pipes_out[1], phase_input, 2 );
        write( pipes_out[1], pipe_input, pipe_read );
        pipe_read = getline( &pipe_input, &pipe_size, output );
        snprintf( phase_input, 2, "%i\n", phase[3] );
        phase_input[1] = '\n';
        write( pipes_out[1], phase_input, 2 );
        write( pipes_out[1], pipe_input, pipe_read );
        pipe_read = getline( &pipe_input, &pipe_size, output );
        snprintf( phase_input, 2, "%i\n", phase[4] );
        phase_input[1] = '\n';
        write( pipes_out[1], phase_input, 2 );
        write( pipes_out[1], pipe_input, pipe_read );
        pipe_read = getline( &pipe_input, &pipe_size, output );
        size_t thrust = strtoul( pipe_input, NULL, 10 );
        size_t phase_num = phase[0] + 10 * phase[1] + 100 * phase[2] +
                           1000 * phase[3] + 10000 * phase[4];
        if ( thrust > maxthrust ) {
            maxthrust = thrust;
            maxphase = phase_num;
        }
        increasePhase( phase );
        phase_num = phase[0] + 10 * phase[1] + 100 * phase[2] +
                    1000 * phase[3] + 10000 * phase[4];
    }
    printf( "INPUT FOR MAX THRUSTERS: %05zu WHICH PRODUCES: %zu\n", maxphase,
            maxthrust );
    close( pipes_in[1] );
    close( pipes_in[0] );
    close( pipes_out[1] );
    close( pipes_out[0] );
    free( code );
    free( input );
}
