#define _GNU_SOURCE
#include <errno.h>
#include <error.h>
#include <fcntl.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

#include "bitmap.h"

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
    ssize_t *tmp = realloc( *code, code_len * 10 * sizeof( ssize_t ) );
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

int getInstruction( ssize_t code, size_t *flags ) {
    int ret = code % 100;
    code /= 100;
    int i = 0;
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
    int instruction = 0;
    size_t actual_code_len = 10 * code_len;
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
            res = strtoimax( input, &end, 10 );
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
            return;
        default:
            error( EXIT_FAILURE, 0,
                   "INCORRECT INSTRUCTION AT INDEX: %zu, INSTRUCTION IS: %li\n",
                   i, code[i] );
        }
        flags = 0;
    }
}

void findExtremes( int **painted, size_t painted_len, int *min_x, int *max_x, int *min_y, int *max_y ) {
    *min_x = *max_x = painted[0][0];
    *min_y = *max_y = painted[0][1];
    for( size_t i = 1; i < painted_len; i++ ) {
        if( *min_x > painted[i][0] )
            *min_x = painted[i][0];
        else if( *max_x < painted[i][0] )
            *max_x = painted[i][0];
        if( *min_y > painted[i][1] )
            *min_y = painted[i][1];
        else if( *max_y < painted[i][1] )
            *max_y = painted[i][1];
    }
}

void printIt( int **painted, size_t painted_len, int min_x, int max_x, int min_y, int max_y ) {
    size_t width = max_x - min_x + 1;
    size_t height = max_y - min_y + 1;
    int *pixels = calloc( width * height, sizeof( int ) );
    if( pixels == NULL )
        error( EXIT_FAILURE, errno, "calloc" );
    for( size_t i = 0; i < painted_len; i++ ) {
        size_t x = painted[i][0];
        size_t y = painted[i][1];
        pixels[(y - min_y)*width + (x-min_x)] = painted[i][2];
    }
    writeBitmap( pixels, width, height );
    for( size_t i = 0; i < height; i++ ) {
        for( size_t j = 0; j < width; j++ ) {
            if( pixels[i*width+j] == 0 )
                putchar( ' ' );
            else
                putchar( '#' );
        }
        putchar( '\n' );
    }
    free( pixels );
}

size_t guideRobot( int in_fd, int out_fd ) {
#ifndef BLACK
    int **painted = malloc( sizeof( int * ) );
    painted[0] = malloc( sizeof( int ) * 3 );
    painted[0][0] = 0;
    painted[0][1] = 0;
    painted[0][2] = 1;
    size_t painted_len = 1;
#else
    int **painted = NULL;
    size_t painted_len = 0;
#endif
    int pos_x = 0;
    int pos_y = 0;
    int move_y = -1;
    int move_x = 0;
    int white = 0;
    size_t cur_square_pos = 0;
    char input[3];
    input[2] = 0;
    while( 1 ) {
        white = 0;
        for( size_t i = 0; i < painted_len; i++ ) {
            if( painted[i][0] == pos_x && painted[i][1] == pos_y ) {
                cur_square_pos = i;
                if( painted[i][2] == 1 )
                    white = 1;
                break;
            }
        }
        char w[3];
        if( white )
            sprintf( w, "1\n" );
        else
            sprintf( w, "0\n" );

        int written = write( out_fd, w, 2 );
        while( written < 2 ) {
            written += write( out_fd, w + written, 2 - written );
        }

        int read_len = read( in_fd, input, 2 );
        if( input[0] == 'E' )
            break;
        while( read_len < 2 ) {
            read_len += read( in_fd, input + read_len, 2 - read_len );
        }
        switch( input[0] ) {
            case '0':
                if( white )
                    painted[cur_square_pos][2] = 0;
                break;
            case '1':
                if( !white ) {
                    if( cur_square_pos < painted_len && painted[cur_square_pos][0] == pos_x && painted[cur_square_pos][1] == pos_y ) {
                        painted[cur_square_pos][2] = 1;
                    } else {
                        int **tmp = realloc( painted, ( painted_len + 1 ) * sizeof( int * ) );
                        if ( tmp == NULL )
                            error( EXIT_FAILURE, 0, "realloc" );
                        painted = tmp;
                        painted[painted_len] = malloc( 3 * sizeof( int ) );
                        if( painted[painted_len] == NULL )
                            error( EXIT_FAILURE, 0, "malloc" );
                        painted[painted_len][0] = pos_x;
                        painted[painted_len][1] = pos_y;
                        painted[painted_len][2] = 1;
                        painted_len++;
                    }
                }
                break;
            default:
                error( EXIT_FAILURE, 0, "UNEXPECTED INPUT '%c'", input[0] );
                break;
        }
        read_len = read( in_fd, input, 2 );
        while( read_len < 2 ) {
            read_len += read( in_fd, input + read_len, 2 - read_len );
        }
        switch( input[0] ) {
            case '0':
                if( move_y != 0 ) {
                    move_x = move_y;
                    move_y = 0;
                } else {
                    move_y = -move_x;
                    move_x = 0;
                }
                break;
            case '1':
                if( move_y != 0 ) {
                    move_x = -move_y;
                    move_y = 0;
                } else {
                    move_y = move_x;
                    move_x = 0;
                }
                break;
            default:
                error( EXIT_FAILURE, 0, "UNEXPECTED INPUT '%c'", input[0] );
                break;
        }
        pos_x += move_x;
        pos_y += move_y;
    }
    int min_x, min_y, max_x, max_y;
    findExtremes( painted, painted_len, &min_x, &max_x, &min_y, &max_y );
    printIt( painted, painted_len, min_x, max_x, min_y, max_y );
    for( size_t i = 0; i < painted_len; i++ ) {
        free( painted[i] );
    }
    free( painted );

    return painted_len;
}

int main() {
    FILE *in = fopen( "input", "r" );
    char *input = NULL;
    size_t input_len = 0;
    getline( &input, &input_len, in );
    ssize_t *code = NULL;
    size_t code_len = populateCode( &code, input );
    int pipes[2];
    pipe( pipes );
    int robot_in = pipes[0];
    int program_out = pipes[1];
    pipe( pipes );
    int robot_out = pipes[1];
    int program_in = pipes[0];
    printf( "STARTING COMPUTING\n" );
    int pid = fork();
    if ( pid > 0 ) {
        if ( dup2( robot_in, STDIN_FILENO ) == -1 )
            error( EXIT_FAILURE, errno, "dup2" );
        if ( dup2( robot_out, STDOUT_FILENO ) == -1 )
            error( EXIT_FAILURE, errno, "dup2" );
        compute( code, code_len );
        printf( "E\n" );
        exit(0);
    } else if ( pid < 0 ) {
        error( EXIT_FAILURE, errno, "fork" );
    }
    printf( "ROBOT PAINTED %zu SQUARES\n",
            guideRobot( program_in, program_out ) );
    free( code );
    free( input );
}
