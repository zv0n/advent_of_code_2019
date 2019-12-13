#define _GNU_SOURCE
#include <errno.h>
#include <error.h>
#include <fcntl.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

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
            printf( "I\n" );
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

void findExtremes( int **pixels, int len_x, int len_y, int *max_x, int *max_y ) {
    *max_x = 0;
    *max_y = 0;
    for( int i = 0; i < len_y; i++ ) {
        for( int j = 0; j < len_x; j++ ) {
            if( pixels[i][j] != 0 ) {
                *max_y = i;
                if( *max_x < j )
                    *max_x = j;
            }
        }
    }
}

void printIt( int **pixels, int max_x, int max_y, bool ret ) {
    int width;
    int height;
    findExtremes( pixels, max_x, max_y, &width, &height );
    for( int i = 0; i <= height; i++ ) {
        for( int j = 0; j <= width; j++ ) {
            if( pixels[i][j] == 0 )
                printf( " " );
            else if( pixels[i][j] == 1 )
                printf( "█" );
            else if( pixels[i][j] == 2 )
                printf( "░" );
            else if( pixels[i][j] == 3 )
                printf( "▬" );
            else
                printf( "•" );
        }
        printf( "\n" );
    }
    if( ret ) {
        printf( "\x1b[%iD\x1b[%iA", width+1, height+1 );
        usleep( 6000 );
    }
}

int playGame( int in_fd, int out_fd ) {
    FILE *f_in = fdopen( in_fd, "r" );
    char *input = NULL;
    size_t input_len = 0;
    int max_x = 128;
    int max_y = 128;
    int score = 0;
    int ball_x = 0;
    int tile_x = 0;
    int **pixels = calloc( max_y, sizeof( int* ) );
    for( int i = 0; i < max_y; i++ ) {
        pixels[i] = calloc( max_x, sizeof( int ) );
    }
    while( 1 ) {
        getline( &input, &input_len, f_in );
        if( input[0] == 'I' ) {
            if( ball_x < tile_x )
                write( out_fd, "-1\n", 3 );
            else if( ball_x > tile_x )
                write( out_fd, "1\n", 2 );
            else
                write( out_fd, "0\n", 2 );
            continue;
        }
        if( input[0] == 'E' )
            break;
        int x = strtoimax( input, NULL, 10 );
        if( x == -1 ) {
            getline( &input, &input_len, f_in );
            getline( &input, &input_len, f_in );
            score = strtoimax( input, NULL, 10 );
            continue;
        }
        getline( &input, &input_len, f_in );
        int y = strtoimax( input, NULL, 10 );
        getline( &input, &input_len, f_in );
        int tile = strtoimax( input, NULL, 10 );
        if( tile == 4 ) {
            ball_x = x;
        } else if ( tile == 3 ) {
            tile_x = x;
        }
        
        if( x >= max_x ) {
            for( int i = 0; i < max_y; i++ ) {
                int *tmp = realloc( pixels[i], (x+1) * sizeof( size_t ) );
                if( tmp == NULL )
                    error( EXIT_FAILURE, errno, "realloc" );
                pixels[i] = tmp;
                for( int j = max_x; j < x+1; j++ ) {
                    pixels[i][j] = 0;
                }
            }
            max_x = x+1;
        }
        if( y >= max_y ) {
            int **tmp = realloc( pixels, (y+1) * sizeof( size_t*  ) );
            if( tmp == NULL )
                error( EXIT_FAILURE, errno, "realloc" );
            pixels = tmp;
            for( int i = max_y; i < y+1; i++ ) {
                pixels[i] = calloc( max_x, sizeof( size_t ) );
            }
            max_y = y+1;
        }

        pixels[y][x] = tile;
        printIt( pixels, max_x, max_y, true );
    }
    printIt( pixels, max_x, max_y, false );
    for( int i = 0; i < max_y; i++ ) {
        free( pixels[i] );
    }
    free( pixels );

    return score;
}

int main() {
    FILE *in = fopen( "input", "r" );
    char *input = NULL;
    size_t input_len = 0;
    getline( &input, &input_len, in );
    ssize_t *code = NULL;
    size_t code_len = populateCode( &code, input );
    code[0] = 2;
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
    printf( "THE FINAL SCORE IS %i\n",
            playGame( program_in, program_out ) );
    free( code );
    free( input );
}
