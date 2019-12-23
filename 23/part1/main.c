#define _GNU_SOURCE
#include <errno.h>
#include <error.h>
#include <fcntl.h>
#include <inttypes.h>
#include <poll.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

//--[ IntCode Computer code ]--------------------------------------------------

#define nic_count 50

// actual
#define PARAM_ACTUAL_0 0x0001
#define PARAM_ACTUAL_1 0x0002
#define PARAM_ACTUAL_2 0x0004
// future proofing
#define PARAM_ACTUAL_3 0x0008
#define PARAM_ACTUAL_4 0x0010
#define PARAM_ACTUAL_5 0x0020
#define PARAM_ACTUAL_6 0x0040
#define PARAM_ACTUAL_7 0x0080
// relative
#define PARAM_RELATIVE_0 0x0100
#define PARAM_RELATIVE_1 0x0200
#define PARAM_RELATIVE_2 0x0400
// future proofing
#define PARAM_RELATIVE_3 0x0800
#define PARAM_RELATIVE_4 0x1000
#define PARAM_RELATIVE_5 0x2000
#define PARAM_RELATIVE_6 0x4000
#define PARAM_RELATIVE_7 0x8000

#define codemultiplier 20

const int param_flags[] = {
    PARAM_ACTUAL_0,   PARAM_ACTUAL_1,   PARAM_ACTUAL_2,   PARAM_ACTUAL_3,
    PARAM_ACTUAL_4,   PARAM_ACTUAL_5,   PARAM_ACTUAL_6,   PARAM_ACTUAL_7,
    PARAM_RELATIVE_0, PARAM_RELATIVE_1, PARAM_RELATIVE_2, PARAM_RELATIVE_3,
    PARAM_RELATIVE_4, PARAM_RELATIVE_5, PARAM_RELATIVE_6, PARAM_RELATIVE_7};

size_t populateCode( ssize_t **code, char *input ) {
    char *end = input;
    size_t array_len = 1024;
    size_t code_len = 0;
    *code = malloc( array_len * sizeof( ssize_t ) );
    if ( code == NULL )
        error( EXIT_FAILURE, errno, "malloc" );
    while ( *end != '\0' && *end != '\n' ) {
        ( *code )[code_len] = strtol( input, &end, 10 );
        input = end + 1;
        code_len++;
        if ( code_len == array_len ) {
            array_len *= 2;
            ssize_t *tmp = realloc( *code, array_len * sizeof( ssize_t ) );
            if ( tmp == NULL )
                error( EXIT_FAILURE, errno, "realloc" );
            *code = tmp;
        }
    }
    ssize_t *tmp = realloc( *code, code_len * codemultiplier * sizeof( ssize_t ) );
    if ( tmp == NULL )
        error( EXIT_FAILURE, errno, "realloc" );
    *code = tmp;
    return code_len;
}

void testOutOfBounds( size_t code_len, size_t position ) {
    if ( position >= code_len )
        error( EXIT_FAILURE, 0, "INSTRUCTION POINTS BEYOND ARRAY, INDEX: %zu\n",
               position );
}

long getInstruction( ssize_t code, size_t *flags ) {
    long ret = code % 100;
    code /= 100;
    long i = 0;
    while ( code > 0 ) {
        if ( code % 10 == 1 )
            *flags |= param_flags[i];
        else if ( code % 10 == 2 )
            *flags |= param_flags[i + 8];
        code /= 10;
        i++;
    }
    return ret;
}

void compute( ssize_t *code, size_t code_len ) {
    ssize_t res = 0;
    ssize_t relative_base = 0;
    ssize_t temp_param = 0;
    char *end = NULL;
    char *input = NULL;
    size_t input_len = 0;
    size_t flags = 0;
    long instruction = 0;
    size_t actual_code_len = codemultiplier * code_len;
    ssize_t index = 0;
    for ( size_t i = 0; i < code_len; ) {
        instruction = getInstruction( code[i], &flags );
        switch ( instruction ) {
        case 1:
        case 2:
            testOutOfBounds( actual_code_len, i + 3 );
            if ( flags & PARAM_ACTUAL_0 ) {
                index = i + 1;
            } else if ( flags & PARAM_RELATIVE_0 ) {
                index = relative_base + code[i + 1];
            } else {
                index = code[i + 1];
            }

            testOutOfBounds( actual_code_len, index );
            res = code[index];

            if ( flags & PARAM_ACTUAL_1 ) {
                index = i + 2;
            } else if ( flags & PARAM_RELATIVE_1 ) {
                index = relative_base + code[i + 2];
            } else {
                index = code[i + 2];
            }

            testOutOfBounds( actual_code_len, index );
            temp_param = code[index];

            if ( instruction == 1 )
                res += temp_param;
            else
                res *= temp_param;
            if ( flags & PARAM_ACTUAL_2 ) {
                error( EXIT_FAILURE, 0,
                       "LAST PARAM MUST BE IN MODE 0 OR 2! Index: %zu", i );
            } else if ( flags & PARAM_RELATIVE_2 ) {
                index = relative_base + code[i + 3];
            } else {
                index = code[i + 3];
            }
            testOutOfBounds( actual_code_len, index );
            code[index] = res;
            i += 4;
            break;
        case 3:
            testOutOfBounds( actual_code_len, i + 1 );
            printf( "I\n" );
            if ( flags & PARAM_ACTUAL_0 ) {
                error( EXIT_FAILURE, 0,
                       "LAST PARAM MUST BE IN MODE 0! Index: %zu", i );
            } else if ( flags & PARAM_RELATIVE_0 ) {
                index = relative_base + code[i + 1];
            } else {
                index = code[i + 1];
            }
            testOutOfBounds( actual_code_len, index );
            getline( &input, &input_len, stdin );
            res = strtol( input, &end, 10 );
            if ( *end != '\n' && *end != '\0' )
                error( EXIT_FAILURE, 0,
                       "GOT INPUT THAT'S NOT AN INTEGER: '%s'\n", input );
            code[index] = res;
            i += 2;
            break;
        case 4:
            testOutOfBounds( actual_code_len, i + 1 );
            if ( flags & PARAM_ACTUAL_0 ) {
                index = i + 1;
            } else if ( flags & PARAM_RELATIVE_0 ) {
                index = relative_base + code[i + 1];
            } else {
                index = code[i + 1];
            }
            testOutOfBounds( actual_code_len, index );
            printf( "%li\n", code[index] );
            i += 2;
            break;
        case 5:
        case 6:
            testOutOfBounds( actual_code_len, i + 2 );
            if ( flags & PARAM_ACTUAL_0 ) {
                index = i + 1;
            } else if ( flags & PARAM_RELATIVE_0 ) {
                index = relative_base + code[i + 1];
            } else {
                index = code[i + 1];
            }
            testOutOfBounds( actual_code_len, index );
            temp_param = code[index];
            if ( ( instruction == 5 && temp_param != 0 ) ||
                 ( instruction == 6 && temp_param == 0 ) ) {
                if ( flags & PARAM_ACTUAL_1 ) {
                    index = i + 2;
                } else if ( flags & PARAM_RELATIVE_1 ) {
                    index = relative_base + code[i + 2];
                } else {
                    index = code[i + 2];
                }
                testOutOfBounds( actual_code_len, index );
                i = code[index];
            } else {
                i += 3;
            }
            break;
        case 7:
        case 8:
            testOutOfBounds( actual_code_len, i + 2 );
            if ( flags & PARAM_ACTUAL_0 ) {
                index = i + 1;
            } else if ( flags & PARAM_RELATIVE_0 ) {
                index = relative_base + code[i + 1];
            } else {
                index = code[i + 1];
            }

            testOutOfBounds( actual_code_len, index );
            res = code[index];

            if ( flags & PARAM_ACTUAL_1 ) {
                index = i + 2;
            } else if ( flags & PARAM_RELATIVE_1 ) {
                index = relative_base + code[i + 2];
            } else {
                index = code[i + 2];
            }

            testOutOfBounds( actual_code_len, index );

            if ( instruction == 7 )
                res = res < code[index] ? 1 : 0;
            else
                res = res == code[index] ? 1 : 0;

            if ( flags & PARAM_ACTUAL_2 ) {
                error( EXIT_FAILURE, 0,
                       "LAST PARAM MUST BE IN MODE 0! Index: %zu", i );
            } else if ( flags & PARAM_RELATIVE_2 ) {
                index = relative_base + code[i + 3];
            } else {
                index = code[i + 3];
            }
            testOutOfBounds( actual_code_len, index );
            code[index] = res;
            i += 4;
            break;
        case 9:
            testOutOfBounds( actual_code_len, i + 1 );
            if ( flags & PARAM_ACTUAL_0 ) {
                index = i + 1;
            } else if ( flags & PARAM_RELATIVE_0 ) {
                index = relative_base + code[i + 1];
            } else {
                index = code[i + 1];
            }
            testOutOfBounds( actual_code_len, index );
            relative_base += code[index];
            i += 2;
            break;
        case 99:
            free( input );
            return;
        default:
            error( EXIT_FAILURE, 0,
                   "INCORRECT INSTRUCTION AT INDEX: %zu, INSTRUCTION IS: %li\n",
                   i, code[i] );
        }
        flags = 0;
    }
}

void communicate( int pipes_array[nic_count][2] ) {
    FILE *file_pointers[nic_count];
    char *input = NULL;
    size_t input_len = 0;
    char buffer[128];
    int packets[nic_count];

    for( long i = 0; i < nic_count; i++ ) {
        fcntl( F_SETFD, pipes_array[i][0], O_NONBLOCK );
        file_pointers[i] = fdopen( pipes_array[i][0], "r" );
        getline( &input, &input_len, file_pointers[i] );
        sprintf( buffer, "%li\n", i );
        write( pipes_array[i][1], buffer, strlen( buffer ) );
        packets[i] = 0;
    }

    while( 1 ) {
        for( int i = 0; i < nic_count; i++ ) {
            if( getline( &input, &input_len, file_pointers[i] ) <= 0 )
                continue;
            if( input[0] == 'E' )
                continue;
            if( input[0] == 'I' ) {
                if( packets[i] == 0 ) {
                    write( pipes_array[i][1], "-1\n", 3 );
                    getline( &input, &input_len, file_pointers[i] );
                    if( input[0] != 'I' )
                        goto sending;
                    write( pipes_array[i][1], "-1\n", 3 );
                } else {
                    getline( &input, &input_len, file_pointers[i] );
                    --packets[i];
                    while( packets[i] > 0 ) {
                        getline( &input, &input_len, file_pointers[i] );
                        getline( &input, &input_len, file_pointers[i] );
                        --packets[i];
                    }
                    getline( &input, &input_len, file_pointers[i] );
                    if( input[0] != 'I' )
                        goto sending;
                    write( pipes_array[i][1], "-1\n", 3 );
                }
            } else {
sending:
                do {
                    long address = strtol( input, NULL, 10 );
                    getline( &input, &input_len, file_pointers[i] );
                    long x = strtol( input, NULL, 10 );
                    getline( &input, &input_len, file_pointers[i] );
                    long y = strtol( input, NULL, 10 );
                    if( address == 255 ) {
                        printf( "X: %li,Y: %li\n", x, y );
                        for( int i = 0; i < nic_count; i++ ) {
                            fclose( file_pointers[i] );
                        }
                        return;
                    }
                    sprintf( buffer, "%li\n", x );
                    write( pipes_array[address][1], buffer, strlen( buffer ) );
                    sprintf( buffer, "%li\n", y );
                    write( pipes_array[address][1], buffer, strlen( buffer ) );
                    ++packets[address];
                    getline( &input, &input_len, file_pointers[i] );
                } while( input[0] != 'I' );
                if( packets[i] == 0 ) {
                    write( pipes_array[i][1], "-1\n", 3 );
                } else {
                    getline( &input, &input_len, file_pointers[i] );
                    --packets[i];
                }
            }
        }
    }
}

int main() {
    FILE *in = fopen( "input", "r" );
    char *input = NULL;
    size_t input_len = 0;
    getline( &input, &input_len, in );
    ssize_t *code = NULL;
    size_t code_len = populateCode( &code, input );
    fclose( in );

    int pipes_array[nic_count][2];
    int pids[nic_count];
    for( int i = 0; i < nic_count; i++ ) {
        int pipes[2];
        pipe( pipes );
        int robot_in = pipes[0];
        int program_out = pipes[1];
        pipe( pipes );
        int robot_out = pipes[1];
        int program_in = pipes[0];
        int pid = fork();
        if ( pid == 0 ) {
            close( program_in );
            close( program_out );
            if ( dup2( robot_in, STDIN_FILENO ) == -1 )
                error( EXIT_FAILURE, errno, "dup2" );
            if ( dup2( robot_out, STDOUT_FILENO ) == -1 )
                error( EXIT_FAILURE, errno, "dup2" );
            setbuf( stdout, NULL );
            compute( code, code_len );
            printf( "END\n" );
            free( code );
            free( input );
            exit( 0 );
        } else if ( pid < 0 ) {
            error( EXIT_FAILURE, errno, "fork" );
        }
        close( robot_out );
        pipes_array[i][0] = program_in;
        pipes_array[i][1] = program_out;
        pids[i] = pid;
    }
    communicate( pipes_array );

    for( int i = 0; i < nic_count; i++ ) {
        close( pipes_array[i][0] );
        close( pipes_array[i][1] );
        kill( pids[i], SIGTERM );
    }
    free( code );
    free( input );
}
