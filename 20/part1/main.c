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

struct queue {
    // 0 - index
    // 1 - path
    int positions[128][2];
    int head;
    int tail;
    int size;
};

void initQueue( struct queue *q ) {
    q->head = 0;
    q->tail = 0;
    q->size = 128;
}

void addToQueue( int pos[2], struct queue *q ) {
    q->positions[q->tail][0] = pos[0];
    q->positions[q->tail][1] = pos[1];
    q->tail++;
    if ( q->tail == q->size )
        q->tail = 0;
    if ( q->tail == q->head )
        error( EXIT_FAILURE, 0, "HEAD == TAIL" );
}

void popFromQueue( int ret[2], struct queue *q ) {
    if ( q->head == q->tail )
        return;
    ret[0] = q->positions[q->head][0];
    ret[1] = q->positions[q->head][1];
    q->head++;
    if ( q->head == q->size )
        q->head = 0;
}

bool queueEmpty( struct queue *q ) {
    return q->head == q->tail;
}

int shortestPath( int **graph, int source_x, int source_y, int target_x,
                  int target_y, int graph_len, int graph_lines ) {
    struct queue q;
    initQueue( &q );
    int cur[2] = {source_y * graph_len + source_x, 0};
    int target_index = target_y*graph_len + target_x;
    int tmp[2] = {0, 0 };
    graph[cur[0]][cur[0]] = -2;
    addToQueue( cur, &q );
    uint64_t shortest = -1;
    while ( !queueEmpty( &q ) ) {
        popFromQueue( cur, &q );
        size_t index = cur[0];
        graph[index][index] = -2;
        if ( cur[0] == target_index ) {
            if ( (size_t)cur[1] < shortest ) {
                shortest = cur[1];
            }
            break;
        }
        for ( size_t i = graph_len; i < (size_t)graph_len * graph_lines; i++ ) {
            if ( i == index )
                continue;
            if ( graph[index][i] != -1 && graph[i][i] != -2 ) {
                uint64_t new_path_weight = cur[1] + graph[index][i];
                if ( new_path_weight < shortest ) {
                    tmp[0] = i;
                    tmp[1] = new_path_weight;
                    addToQueue( tmp, &q );
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
            if( input_graph[i][j] == ' ' && j > 2 && i > 2 && j < input_len - 3 && inner_donut == (size_t)-1 )
                inner_donut = j;
            if ( input_graph[i][j] >= 65 && input_graph[i][j] <= 90 ) {
                int first_letter = 0, second_letter = 0;
                int pos_x = 0, pos_y = 0;
                if ( input_graph[i - 1][j] >= 65 &&
                     input_graph[i - 1][j] <= 90 ) {
                    first_letter = input_graph[i - 1][j] - 65;
                    second_letter = input_graph[i][j] - 65;
                    pos_x = j;
                    pos_y = i+1;
                    if( (size_t)pos_y == inner_donut + 2 || (size_t)pos_y == input_lines )
                        pos_y -= 3;
                } else if ( input_graph[i + 1][j] >= 65 &&
                            input_graph[i + 1][j] <= 90 ) {
                    first_letter = input_graph[i][j] - 65;
                    second_letter = input_graph[i + 1][j] - 65;
                    pos_x = j;
                    pos_y = i + 2;
                    if( (size_t)pos_y == inner_donut + 2 || (size_t)pos_y == input_lines )
                        pos_y -= 3;
                } else if ( input_graph[i][j - 1] >= 65 &&
                            input_graph[i][j - 1] <= 90 ) {
                    first_letter = input_graph[i][j - 1] - 65;
                    second_letter = input_graph[i][j] - 65;
                    pos_x = j+1;
                    pos_y = i;
                    if( (size_t)pos_x == inner_donut + 2 || (size_t)pos_x == input_len )
                        pos_x -= 3;
                } else {
                    first_letter = input_graph[i][j] - 65;
                    second_letter = input_graph[i][j + 1] - 65;
                    pos_x = j + 2;
                    pos_y = i;
                    if( (size_t)pos_x == inner_donut + 2 || (size_t)pos_x == input_len )
                        pos_x -= 3;
                }
                size_t index = first_letter * 26 + second_letter;
                if ( portals[index][0] == 0 ) {
                    portals[index][0] = pos_x;
                    portals[index][1] = pos_y;
                } else if( portals[index][0] != pos_x || portals[index][1] != pos_y ) {
                    portals[index][2] = pos_x;
                    portals[index][3] = pos_y;
                }
            }
        }
    }

    int **graph = malloc( input_lines * input_len * sizeof( int * ) );
    for ( uint64_t i = 0; i < input_len * input_lines; i++ ) {
        graph[i] = malloc( input_lines * input_len * sizeof( int ) );
        for ( size_t j = 0; j < input_len * input_lines; j++ ) {
            graph[i][j] = -1;
        }
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
            graph[index1][index2] = 1;
            graph[index2][index1] = 1;
        }
    }

    printf( "SHORTEST PATH FROM AA TO ZZ IS: %i\n",
            shortestPath( graph, start_x, start_y, target_x, target_y,
                          input_len, input_lines ) );
}
