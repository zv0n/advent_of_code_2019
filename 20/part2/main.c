#define _GNU_SOURCE
#include <ctype.h>
#include <errno.h>
#include <error.h>
#include <fcntl.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#define graph_info_size 10
#define depth0 0x0000000000000001
#define depth1 0x0000000000000002
#define depth2 0x0000000000000004
#define depth3 0x0000000000000008
#define depth4 0x0000000000000010
#define depth5 0x0000000000000020
#define depth6 0x0000000000000040
#define depth7 0x0000000000000080
#define depth8 0x0000000000000100
#define depth9 0x0000000000000200
#define depth10 0x0000000000000400
#define depth11 0x0000000000000800
#define depth12 0x0000000000001000
#define depth13 0x0000000000002000
#define depth14 0x0000000000004000
#define depth15 0x0000000000008000
#define depth16 0x0000000000010000
#define depth17 0x0000000000020000
#define depth18 0x0000000000040000
#define depth19 0x0000000000080000
#define depth20 0x0000000000100000
#define depth21 0x0000000000200000
#define depth22 0x0000000000400000
#define depth23 0x0000000000800000
#define depth24 0x0000000001000000
#define depth25 0x0000000002000000
#define depth26 0x0000000004000000
#define depth27 0x0000000008000000
#define depth28 0x0000000010000000
#define depth29 0x0000000020000000
#define depth30 0x0000000040000000
#define depth31 0x0000000080000000
#define depth32 0x0000000100000000
#define depth33 0x0000000200000000
#define depth34 0x0000000400000000
#define depth35 0x0000000800000000
#define depth36 0x0000001000000000
#define depth37 0x0000002000000000
#define depth38 0x0000004000000000
#define depth39 0x0000008000000000
#define depth40 0x0000010000000000
#define depth41 0x0000020000000000
#define depth42 0x0000040000000000
#define depth43 0x0000080000000000
#define depth44 0x0000100000000000
#define depth45 0x0000200000000000
#define depth46 0x0000400000000000
#define depth47 0x0000800000000000
#define depth48 0x0001000000000000
#define depth49 0x0002000000000000
#define depth50 0x0004000000000000
#define depth51 0x0008000000000000
#define depth52 0x0010000000000000
#define depth53 0x0020000000000000
#define depth54 0x0040000000000000
#define depth55 0x0080000000000000
#define depth56 0x0100000000000000
#define depth57 0x0200000000000000
#define depth58 0x0400000000000000
#define depth59 0x0800000000000000
#define depth60 0x1000000000000000
#define depth61 0x2000000000000000
#define depth62 0x4000000000000000
#define depth63 0x8000000000000000

const uint64_t depth_flags[64] = {
    depth0, depth1, depth2,  depth3,  depth4,  depth5,  depth6,  depth7,
    depth8, depth9, depth10, depth11, depth12, depth13, depth14, depth15,
    depth16, depth17, depth18, depth19, depth20, depth21, depth22, depth23,
    depth24, depth25, depth26, depth27, depth28, depth29, depth30, depth31,
    depth32, depth33, depth34, depth35, depth36, depth37, depth38, depth39,
    depth40, depth41, depth42, depth43, depth44, depth45, depth46, depth47,
    depth48, depth49, depth50, depth51, depth52, depth53, depth54, depth55,
    depth56, depth57, depth58, depth59, depth60, depth61, depth62, depth63};

struct queue {
    // 0 - index
    // 1 - path length
    // 2 - level
    int positions[256][3];
    int head;
    int tail;
    int size;
};

void initQueue( struct queue *q ) {
    q->head = 0;
    q->tail = 0;
    q->size = 256;
}

size_t queueSize( struct queue *q ) {
    int a = q->tail - q->head;
    if( a < 0 )
        return a + q->size;
    return a;
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

bool visited( uint64_t **graph, size_t index, int depth ) {
    if( depth > (64*graph_info_size)-1 )
        error( EXIT_FAILURE, 0, "DEPTH TOO DEEP\n" );
    int depth_index = depth / 64;
    depth -= depth_index*64;
    return graph[depth_index][index] & depth_flags[depth];
}

void visit( uint64_t **graph, size_t index, int depth ) {
    int depth_index = depth / 64;
    if( depth_index >= graph_info_size )
        error( EXIT_FAILURE, 0, "DEPTH TOO DEEP\n" );
    depth -= depth_index*64;
    graph[depth_index][index] |= depth_flags[depth];
}

int shortestPath( int **graph, uint64_t **graph_info, int source_x, int source_y, int target_x,
                  int target_y, int graph_len, int graph_lines ) {
    struct queue q;
    initQueue( &q );
    int cur[3] = {source_y * graph_len + source_x, 0, 0};
    int tmp[3] = {0, 0, 0};
    size_t target_index = target_y * graph_len + target_x;
    addToQueue( cur, &q );
    visit( graph_info, cur[0], cur[2] );
    uint64_t shortest = -1;
    while ( !queueEmpty( &q ) ) {
        popFromQueue( cur, &q );
        size_t index = cur[0];
        if ( (size_t)cur[0] == target_index && cur[2] == 0 ) {
            if ( (size_t)cur[1] < shortest ) {
                shortest = cur[1];
            }
            break;
        }
        for ( size_t i = graph_len; i < (size_t)graph_len * graph_lines; i++ ) {
            if ( i == index )
                continue;
            if ( graph[index][i] != 0 &&
                 ( ( cur[2] == 0 &&
                     ( graph[index][i] == 1 || graph[index][i] == -1 ) ) ||
                   ( cur[2] > 0 && i != target_index ) ) ) {
                uint64_t new_path_weight = cur[1] + 1;
                if ( new_path_weight < shortest ) {
                    tmp[0] = i;
                    tmp[1] = new_path_weight;
                    if ( graph[index][i] == 1 ) {
                        tmp[2] = cur[2];
                    } else if ( graph[index][i] == -1 ) {
                        tmp[2] = cur[2] + 1;
                    } else if ( graph[index][i] == -2 ) {
                        tmp[2] = cur[2] - 1;
                    }
                    if( !visited( graph_info, tmp[0], tmp[2] ) ) {
                        addToQueue( tmp, &q );
                        visit( graph_info, tmp[0], tmp[2] );
                    }
                }
            }
        }
    }
    return shortest;
}

int main( void ) {
    FILE *in = fopen( "input", "r" );
    char *input = NULL;
    uint64_t input_buf = 0;
    uint64_t input_lines = 0;
    uint64_t input_len = getline( &input, &input_buf, in );
    input_len--;
    char **input_graph = malloc( sizeof( char * ) );
    // first line is just stone walls, ne need to do anything with it
    input_graph[0] = malloc( input_len );
    memcpy( input_graph[0], input, input_len );
    input_lines++;
    while ( getline( &input, &input_buf, in ) > 0 ) {
        void *tmp =
            realloc( input_graph, sizeof( char * ) * ( input_lines + 1 ) );
        if ( tmp == NULL )
            error( EXIT_FAILURE, errno, "realloc" );
        input_graph = tmp;
        input_graph[input_lines] = malloc( input_len );
        memcpy( input_graph[input_lines], input, input_len );
        input_lines++;
    }

    int portals[676][4];

    for ( int i = 0; i < 676; i++ ) {
        portals[i][0] = 0;
        portals[i][1] = 0;
    }

    size_t inner_donut = -1;

    for ( size_t i = 1; i < input_lines - 1; i++ ) {
        for ( size_t j = 1; j < input_len - 1; j++ ) {
            if ( input_graph[i][j] == ' ' && j > 2 && i > 2 &&
                 j < input_len - 3 && inner_donut == (size_t)-1 )
                inner_donut = j;
            if ( input_graph[i][j] >= 65 && input_graph[i][j] <= 90 ) {
                int first_letter = 0, second_letter = 0;
                int pos_x = 0, pos_y = 0;
                if ( input_graph[i - 1][j] >= 65 &&
                     input_graph[i - 1][j] <= 90 ) {
                    first_letter = input_graph[i - 1][j] - 65;
                    second_letter = input_graph[i][j] - 65;
                    pos_x = j;
                    pos_y = i + 1;
                    if ( (size_t)pos_y == inner_donut + 2 ||
                         (size_t)pos_y == input_lines )
                        pos_y -= 3;
                } else if ( input_graph[i + 1][j] >= 65 &&
                            input_graph[i + 1][j] <= 90 ) {
                    first_letter = input_graph[i][j] - 65;
                    second_letter = input_graph[i + 1][j] - 65;
                    pos_x = j;
                    pos_y = i + 2;
                    if ( (size_t)pos_y == inner_donut + 2 ||
                         (size_t)pos_y == input_lines )
                        pos_y -= 3;
                } else if ( input_graph[i][j - 1] >= 65 &&
                            input_graph[i][j - 1] <= 90 ) {
                    first_letter = input_graph[i][j - 1] - 65;
                    second_letter = input_graph[i][j] - 65;
                    pos_x = j + 1;
                    pos_y = i;
                    if ( (size_t)pos_x == inner_donut + 2 ||
                         (size_t)pos_x == input_len )
                        pos_x -= 3;
                } else {
                    first_letter = input_graph[i][j] - 65;
                    second_letter = input_graph[i][j + 1] - 65;
                    pos_x = j + 2;
                    pos_y = i;
                    if ( (size_t)pos_x == inner_donut + 2 ||
                         (size_t)pos_x == input_len )
                        pos_x -= 3;
                }
                size_t index = first_letter * 26 + second_letter;
                if ( portals[index][0] == 0 ) {
                    portals[index][0] = pos_x;
                    portals[index][1] = pos_y;
                } else if ( portals[index][0] != pos_x ||
                            portals[index][1] != pos_y ) {
                    portals[index][2] = pos_x;
                    portals[index][3] = pos_y;
                }
            }
        }
    }

    int **graph = malloc( input_lines * input_len * sizeof( int * ) );
    for ( uint64_t i = 0; i < input_len * input_lines; i++ ) {
        graph[i] = calloc( input_lines * input_len, sizeof( int ) );
    }

    for ( uint64_t i = 1; i < input_lines - 1; i++ ) {
        for ( uint64_t j = 1; j < input_len - 1; j++ ) {
            if ( input_graph[i][j] != '#' && input_graph[i][j] != ' ' ) {
                if ( input_graph[i - 1][j] == '.' )
                    graph[i * input_len + j][( i - 1 ) * input_len + j] = 1;

                if ( input_graph[i + 1][j] == '.' )
                    graph[i * input_len + j][( i + 1 ) * input_len + j] = 1;

                if ( input_graph[i][j - 1] == '.' )
                    graph[i * input_len + j][i * input_len + j - 1] = 1;

                if ( input_graph[i][j + 1] == '.' )
                    graph[i * input_len + j][i * input_len + j + 1] = 1;
            }
            graph[i * input_len + j][i * input_len + j] = -1;
        }
    }

    int start_x = portals[0][0];
    int start_y = portals[0][1];
    int target_x = portals[675][0];
    int target_y = portals[675][1];

    for ( int i = 1; i < 675; i++ ) {
        if ( portals[i][0] != 0 ) {
            size_t index1 = portals[i][1] * input_len + portals[i][0];
            size_t index2 = portals[i][3] * input_len + portals[i][2];
            if ( portals[i][0] == 2 || portals[i][1] == 2 ||
                 (size_t)portals[i][0] == input_len - 3 ||
                 (size_t)portals[i][1] == input_lines - 3 ) {
                // outer portal
                graph[index1][index2] = -2;
                graph[index2][index1] = -1;
            } else {
                // inner portal
                graph[index1][index2] = -1;
                graph[index2][index1] = -2;
            }
        }
    }

    uint64_t **graph_info = malloc( graph_info_size * sizeof( uint64_t * ) );
    for( int i = 0; i < 10; i++ ) {
        graph_info[i] = calloc( input_lines * input_len, sizeof( uint64_t ) );
    }

    printf( "SHORTEST PATH FROM AA TO ZZ IS: %i\n",
            shortestPath( graph, graph_info, start_x, start_y, target_x, target_y,
                          input_len, input_lines ) );
}
