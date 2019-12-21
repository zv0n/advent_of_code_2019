#define _GNU_SOURCE
#include <errno.h>
#include <error.h>
#include <fcntl.h>
#include <inttypes.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

//--[ IntCode Computer code ]--------------------------------------------------

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
            if ( flags & PARAM_ACTUAL_0 ) {
                error( EXIT_FAILURE, 0,
                       "LAST PARAM MUST BE IN MODE 0! Index: %zu", i );
            } else if ( flags & PARAM_RELATIVE_0 ) {
                index = relative_base + code[i + 1];
            } else {
                index = code[i + 1];
            }
            testOutOfBounds( actual_code_len, index );
            printf( "I\n" );
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

//--[ Printing ]---------------------------------------------------------------

void printIt( char **lines, int lines_num ) {
    for ( int i = 0; i <= lines_num; i++ ) {
        printf( "%s\n", lines[i] );
    }
}

int intersections( char **lines, int lines_num, int line_len ) {
    int ret = 0;
    for ( int i = 0; i < lines_num; i++ ) {
        for ( int j = 0; j < line_len; j++ ) {
            if ( lines[i][j] == '#' ) {
                // there's # above
                if ( i - 1 > 0 && lines[i - 1][j] == '#' ) {
                    // there's a # below
                    if ( i + 1 < lines_num && lines[i + 1][j] == '#' ) {
                        // there's a # on left or right
                        if ( ( j - 1 > 0 && lines[i][j - 1] == '#' ) ||
                             ( j + 1 < line_len && lines[i][j + 1] == '#' ) ) {
                            ret += i * j;
                            lines[i][j] = 'O';
                        }
                    } else {
                        // there isn't a # below and there are # on either side
                        // of current position
                        if ( j - 1 > 0 && lines[i][j - 1] == '#' &&
                             j + 1 < line_len && lines[i][j + 1] == '#' ) {
                            ret += i * j;
                            lines[i][j] = 'O';
                        }
                    }
                } else {
                    // there isn't # above, but there are # on all other sides
                    if ( i + 1 < lines_num && lines[i + 1][j] == '#' &&
                         j - 1 > 0 && lines[i][j - 1] == '#' &&
                         j + 1 < line_len && lines[i][j + 1] == '#' ) {
                        ret += i * j;
                        lines[i][j] = 'O';
                    }
                }
            }
        }
    }
    return ret;
}

void insertCode( int in_fd, int out_fd ) {
    FILE *f_in = fdopen( in_fd, "r" );
    char *input = NULL;
    size_t input_len = 0;
    char buffer[16];
    int res = 0;
    while( getline( &input, &input_len, f_in ) ) {
        res = strtoimax( input, NULL, 10 );
        printf( "%c", res );
        if( !strcmp( input, "10\n" ) )
            break;
    }
    putchar( '\n' );

    while( getline( &input, &input_len, stdin ) ) {
        for( size_t i = 0 ; i < strlen( input ); i++ ) {
            sprintf( buffer, "%i\n", (int)input[i] );
            write( out_fd, buffer, strlen( buffer ) );
        }
        if( !strcmp( input, "WALK\n" ) || !strcmp( input, "RUN\n" ) )
            break;
    }

    while( getline( &input, &input_len, f_in ) ) {
        if( !strcmp( input, "END\n" ) )
            break;
        res = strtoimax( input, NULL, 10 );
        if( res < 255 ) {
            printf( "%c", res );
        } else {
            printf( "HULL DAMAGE IS: %i\n", res );
        }
    }

    fclose( f_in );
}

int main() {
    FILE *in = fopen( "input", "r" );
    char *input = NULL;
    size_t input_len = 0;
    getline( &input, &input_len, in );
    ssize_t *code = NULL;
    size_t code_len = populateCode( &code, input );
    fclose( in );
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
    insertCode( program_in, program_out );

    close( program_in );
    close( program_out );
    kill( pid, SIGTERM );
    free( code );
    free( input );
}
