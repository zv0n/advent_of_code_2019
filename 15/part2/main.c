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

//--[ Printing ]---------------------------------------------------------------

void findExtremes( int **pixels, int len_x, int len_y, int *max_x,
                   int *max_y ) {
    *max_x = 0;
    *max_y = 0;
    for ( int i = 0; i < len_y; i++ ) {
        for ( int j = 0; j < len_x; j++ ) {
            if ( pixels[i][j] != 0 ) {
                *max_y = i;
                if ( *max_x < j )
                    *max_x = j;
            }
        }
    }
}

void printIt( int **pixels, int max_x, int max_y, bool ret ) {
    int width;
    int height;
    findExtremes( pixels, max_x, max_y, &width, &height );
    for ( int i = 0; i <= height; i++ ) {
        for ( int j = 0; j <= width; j++ ) {
            if ( pixels[i][j] == 0 )
                printf( "░" );
            else if ( pixels[i][j] == 1 )
                printf( "█" );
            else if ( pixels[i][j] == 2 )
                printf( " " );
            else if ( pixels[i][j] == 3 )
                printf( "▬" );
            else
                printf( "•" );
        }
        printf( "\n" );
    }
    if ( ret ) {
        printf( "\x1b[%iD\x1b[%iA", width + 1, height + 1 );
        usleep( 2000 );
    }
}

//--[ Queue Operation ]--------------------------------------------------------

struct queue {
    int positions[128][3];
    int head;
    int tail;
    int size;
};

void initQueue( struct queue *q ) {
    q->head = 0;
    q->tail = 0;
    q->size = 128;
}

void addToQueue( int pos[3], struct queue *q ) {
    q->positions[q->tail][0] = pos[0];
    q->positions[q->tail][1] = pos[1];
    q->positions[q->tail][2] = pos[2];
    q->tail++;
    if ( q->tail == q->size )
        q->tail = 0;
    if ( q->tail == q->head )
        error( EXIT_FAILURE, 0, "HEAD == TAIL" );
}

void popFromQueue( int ret[3], struct queue *q ) {
    if ( q->head == q->tail )
        return;
    ret[0] = q->positions[q->head][0];
    ret[1] = q->positions[q->head][1];
    ret[2] = q->positions[q->head][2];
    q->head++;
    if ( q->head == q->size )
        q->head = 0;
}

bool queueEmpty( struct queue *q ) {
    return q->head == q->tail;
}

//--[ Maze operations ]--------------------------------------------------------

bool fullMaze( int **pixels, int max_x, int max_y ) {
    for ( int i = 1; i < max_y - 1; i++ ) {
        for ( int j = 1; j < max_x - 1; j++ ) {
            if ( pixels[i][j] == 2 ) {
                if ( pixels[i - 1][j] == 0 || pixels[i + 1][j] == 0 ||
                     pixels[i][j - 1] == 0 || pixels[i][j + 1] == 0 )
                    return false;
            }
        }
    }
    return true;
}

bool emptyMaze( int **pixels, int max_x, int max_y ) {
    for ( int i = 1; i < max_y - 1; i++ ) {
        for ( int j = 1; j < max_x - 1; j++ ) {
            if ( pixels[i][j] == 2 ) {
                return false;
            }
        }
    }
    return true;
}

int findShortestWay( int in_fd, int out_fd ) {
    printf( "\x1b[?25l" );
    FILE *f_in = fdopen( in_fd, "r" );
    char *input = NULL;
    size_t input_len = 0;
    int max_x = 64;
    int max_y = 64;
    int cur_x = 21;
    int cur_y = 21;
    int next_x = 0;
    int next_y = 0;
    int **pixels = calloc( max_y, sizeof( int * ) );
    for ( int i = 0; i < max_y; i++ ) {
        pixels[i] = calloc( max_x, sizeof( int ) );
    }
    int next_dir = 1;
    pixels[cur_y][cur_x] = 2;
    while ( !fullMaze( pixels, max_x, max_y ) ) {
        int prevval = pixels[cur_y][cur_x];
        pixels[cur_y][cur_x] = 4;
        pixels[cur_y][cur_x] = prevval;
        switch ( next_dir ) {
        case 1:
            write( out_fd, "1\n", 2 );
            next_y = cur_y - 1;
            next_x = cur_x;
            break;
        case 2:
            write( out_fd, "2\n", 2 );
            next_y = cur_y + 1;
            next_x = cur_x;
            break;
        case 3:
            write( out_fd, "3\n", 2 );
            next_x = cur_x - 1;
            next_y = cur_y;
            break;
        default:
            write( out_fd, "4\n", 2 );
            next_x = cur_x + 1;
            next_y = cur_y;
            break;
        }
        getline( &input, &input_len, f_in );
        int status = strtoimax( input, NULL, 10 );

        switch ( status ) {
        case 1:
            pixels[next_y][next_x] = 2;
            cur_x = next_x;
            cur_y = next_y;
            /* FALLTHROUGH */
        case 0:
            if ( status == 0 )
                pixels[next_y][next_x] = 1;
            if ( pixels[cur_y][cur_x + 1] == 0 ) {
                next_dir = 4;
            } else if ( pixels[cur_y + 1][cur_x] == 0 ) {
                next_dir = 2;
            } else if ( pixels[cur_y][cur_x - 1] == 0 ) {
                next_dir = 3;
            } else if ( pixels[cur_y - 1][cur_x] == 0 ) {
                next_dir = 1;
            } else if ( status == 0 ) {
                switch ( next_dir ) {
                case 1:
                    if ( pixels[cur_y][cur_x + 1] != 1 )
                        next_dir = 4;
                    else if ( pixels[cur_y][cur_x - 1] != 1 )
                        next_dir = 3;
                    else
                        next_dir = 2;
                    break;
                case 2:
                    if ( pixels[cur_y][cur_x - 1] != 1 )
                        next_dir = 3;
                    else if ( pixels[cur_y][cur_x + 1] != 1 )
                        next_dir = 4;
                    else
                        next_dir = 1;
                    break;
                case 3:
                    if ( pixels[cur_y - 1][cur_x] != 1 )
                        next_dir = 1;
                    else if ( pixels[cur_y + 1][cur_x] != 1 )
                        next_dir = 2;
                    else
                        next_dir = 4;
                    break;
                default:
                    if ( pixels[cur_y + 1][cur_x] != 1 )
                        next_dir = 2;
                    else if ( pixels[cur_y - 1][cur_x] != 1 )
                        next_dir = 1;
                    else
                        next_dir = 3;
                    break;
                }
            } else {
                switch ( next_dir ) {
                case 1:
                    if ( pixels[cur_y][cur_x - 1] == 2 )
                        next_dir = 3;
                    break;
                case 2:
                    if ( pixels[cur_y][cur_x + 1] == 2 )
                        next_dir = 4;
                    break;
                case 3:
                    if ( pixels[cur_y + 1][cur_x] == 2 )
                        next_dir = 2;
                    break;
                case 4:
                    if ( pixels[cur_y - 1][cur_x] == 2 )
                        next_dir = 1;
                    break;
                }
            }
            break;
        default:
            pixels[next_y][next_x] = 3;
            cur_x = next_x;
            cur_y = next_y;
            break;
        }
    }
    int **pixels_backup = malloc( max_y * sizeof( int * ) );
    for ( int i = 0; i < max_y; i++ ) {
        pixels_backup[i] = malloc( max_x * sizeof( int ) );
        memcpy( pixels_backup[i], pixels[i], max_x * sizeof( int ) );
    }
    int pos[3] = {21, 21, 0};
    int tmppos[3] = {0, 0, 0};
    struct queue q;
    initQueue( &q );
    addToQueue( pos, &q );
    int steps = 0;
    while ( !queueEmpty( &q ) ) {
        popFromQueue( pos, &q );
        if ( pixels[pos[1]][pos[0]] == 3 ) {
            steps = pos[2];
            break;
        }
        pixels[pos[1]][pos[0]] = 0;
        if ( pixels[pos[1]][pos[0] + 1] >= 2 ) {
            tmppos[0] = pos[0] + 1;
            tmppos[1] = pos[1];
            tmppos[2] = pos[2] + 1;
            addToQueue( tmppos, &q );
        }
        if ( pixels[pos[1]][pos[0] - 1] >= 2 ) {
            tmppos[0] = pos[0] - 1;
            tmppos[1] = pos[1];
            tmppos[2] = pos[2] + 1;
            addToQueue( tmppos, &q );
        }
        if ( pixels[pos[1] + 1][pos[0]] >= 2 ) {
            tmppos[0] = pos[0];
            tmppos[1] = pos[1] + 1;
            tmppos[2] = pos[2] + 1;
            addToQueue( tmppos, &q );
        }
        if ( pixels[pos[1] - 1][pos[0]] >= 2 ) {
            tmppos[0] = pos[0];
            tmppos[1] = pos[1] - 1;
            tmppos[2] = pos[2] + 1;
            addToQueue( tmppos, &q );
        }
        int prevval = pixels[pos[1]][pos[0]];
        pixels[pos[1]][pos[0]] = 4;
        printIt( pixels, max_x, max_y, true );
        pixels[pos[1]][pos[0]] = prevval;
    }
    for ( int i = 0; i < max_y; i++ ) {
        free( pixels[i] );
    }
    free( pixels );

    initQueue( &q );
    pos[2] = 0;
    addToQueue( pos, &q );
    while ( !emptyMaze( pixels_backup, max_x, max_y ) && !queueEmpty( &q ) ) {
        popFromQueue( pos, &q );
        pixels_backup[pos[1]][pos[0]] = 0;
        if ( pixels_backup[pos[1]][pos[0] + 1] >= 2 ) {
            tmppos[0] = pos[0] + 1;
            tmppos[1] = pos[1];
            tmppos[2] = pos[2] + 1;
            addToQueue( tmppos, &q );
        }
        if ( pixels_backup[pos[1]][pos[0] - 1] >= 2 ) {
            tmppos[0] = pos[0] - 1;
            tmppos[1] = pos[1];
            tmppos[2] = pos[2] + 1;
            addToQueue( tmppos, &q );
        }
        if ( pixels_backup[pos[1] + 1][pos[0]] >= 2 ) {
            tmppos[0] = pos[0];
            tmppos[1] = pos[1] + 1;
            tmppos[2] = pos[2] + 1;
            addToQueue( tmppos, &q );
        }
        if ( pixels_backup[pos[1] - 1][pos[0]] >= 2 ) {
            tmppos[0] = pos[0];
            tmppos[1] = pos[1] - 1;
            tmppos[2] = pos[2] + 1;
            addToQueue( tmppos, &q );
        }
        int prevval = pixels_backup[pos[1]][pos[0]];
        pixels_backup[pos[1]][pos[0]] = 4;
        printIt( pixels_backup, max_x, max_y, true );
        pixels_backup[pos[1]][pos[0]] = prevval;
    }
    printIt( pixels_backup, max_x, max_y, false );
    printf( "SHORTEST WAY TO LEAK IS: %i\n", steps );
    printf( "OXYGEN TAKES %i MINUTES TO FILL THE SHIP\n", pos[2] );
    printf( "\x1b[?25h" );
    for ( int i = 0; i < max_y; i++ ) {
        free( pixels_backup[i] );
    }
    free( pixels_backup );
    free( input );
    fclose( f_in );

    return steps;
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
        printf( "E\n" );
        exit( 0 );
    } else if ( pid < 0 ) {
        error( EXIT_FAILURE, errno, "fork" );
    }
    close( robot_in );
    close( robot_out );
    findShortestWay( program_in, program_out );
    kill( pid, SIGTERM );
    free( code );
    free( input );
    fclose( in );
}
