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

#define MAXBEST 3136

struct cache {
    uint16_t *best;
    uint16_t count;
};

void addToCache(struct cache *cache, int index, uint16_t value) {
    cache->count++;
    void *tmp = realloc(cache->best, cache->count * 2 * sizeof(uint16_t));
    if( tmp == NULL )
        error(1, errno, "realloc");
    cache->best = tmp;
    cache->best[cache->count*2 - 2] = index;
    cache->best[cache->count*2 - 1] = value;
}

uint16_t getFromCache(struct cache *cache, int index) {
    for(uint16_t i = 0; i < cache->count; i++) {
        if(cache->best[i*2] == index)
            return cache->best[i*2+1];
    }
    return 0;
}

void initCache(struct cache *cache) {
    cache->count = 0;
    cache->best = NULL;
}

struct cache **globCache = NULL;

int cacheBestMult[4] = {0,0,0,0};

int bestIndex(int pos[4], int keymap[26]) {
    int ret = 0;
    ret += ((pos[0] < 0 ? -1 : keymap[pos[0]])+1) * cacheBestMult[0];
    ret += ((pos[1] < 0 ? -1 : keymap[pos[1]])+1) * cacheBestMult[1];
    ret += ((pos[2] < 0 ? -1 : keymap[pos[2]])+1) * cacheBestMult[2];
    ret += ((pos[3] < 0 ? -1 : keymap[pos[3]])+1) * cacheBestMult[3];
    return ret;
}

struct helpingStruct {
    bool visited;
    size_t required[26][2];
    size_t required_count;
    size_t path_len;
};

//--[ Queue Operation ]--------------------------------------------------------

struct queue {
    uint64_t positions[128][2];
    int head;
    int tail;
    int size;
};

void initQueue( struct queue *q ) {
    q->head = 0;
    q->tail = 0;
    q->size = 128;
}

void addToQueue( uint64_t pos[2], struct queue *q ) {
    q->positions[q->tail][0] = pos[0];
    q->positions[q->tail][1] = pos[1];
    q->tail++;
    if ( q->tail == q->size )
        q->tail = 0;
    if ( q->tail == q->head )
        error( EXIT_FAILURE, 0, "HEAD == TAIL" );
}

void popFromQueue( uint64_t ret[2], struct queue *q ) {
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

//--[ Shortest Path operations ]-----------------------------------------------

void coordToPos( size_t pos[2], size_t x, size_t y ) {
    pos[0] = x;
    pos[1] = y;
}

void testAddGraph( char **graph, size_t y, size_t x, struct queue *q,
                   struct helpingStruct *graphInfoSource,
                   struct helpingStruct *graphInfoTarget, int input_len,
                   int input_lines ) {
    uint64_t tmp[2];
    if ( graph[y][x] != '#' && !graphInfoTarget->visited ) {
        coordToPos( tmp, x, y );
        addToQueue( tmp, q );
        graphInfoTarget->path_len = graphInfoSource->path_len + 1;
        for ( int i = 0; i < 26; i++ ) {
            memcpy( graphInfoTarget->required[i], graphInfoSource->required[i],
                    2 * sizeof( size_t ) );
        }
        graphInfoTarget->required_count = graphInfoSource->required_count;
        graphInfoTarget->visited = true;
        if ( graph[y][x] >= 65 && graph[y][x] <= 90 ) {
            size_t search_x = -1, search_y = -1;
            char searched_key = graph[y][x] + 32;
            for ( int i = 0; i < input_lines; i++ ) {
                for ( int j = 0; j < input_len; j++ ) {
                    if ( graph[i][j] == searched_key ) {
                        search_x = j;
                        search_y = i;
                    }
                }
            }
            graphInfoTarget->required[graphInfoTarget->required_count][0] =
                search_x;
            graphInfoTarget->required[graphInfoTarget->required_count][1] =
                search_y;
            graphInfoTarget->required_count++;
        }
    }
}

bool dependenciesMet( int *requirements, int found_keys[26] ) {
    if ( requirements == NULL )
        return false;
    while ( *requirements != -1 ) {
        if ( found_keys[*requirements] == 0 )
            return false;
        requirements++;
    }
    return true;
}

void resetGraph( struct helpingStruct **graph_info, int input_len,
                 int input_lines ) {
    for ( int i = 0; i < input_lines; i++ ) {
        for ( int j = 0; j < input_len; j++ ) {
            graph_info[i][j].visited = false;
            graph_info[i][j].path_len = 0;
            graph_info[i][j].required_count = 0;
        }
    }
}

size_t graphPath( char **graph, int input_len, int input_lines, size_t start_x,
                  size_t start_y, size_t target_x, size_t target_y,
                  struct helpingStruct **graph_info, int **requirements,
                  size_t keys[26][3] ) {
    struct queue q;
    initQueue( &q );
    size_t pos[2] = {start_x, start_y};
    addToQueue( pos, &q );
    while ( !queueEmpty( &q ) ) {
        popFromQueue( pos, &q );
        uint64_t x = pos[0];
        uint64_t y = pos[1];
        if ( x == target_x && y == target_y ) {
            break;
        }
        testAddGraph( graph, y - 1, x, &q, &graph_info[y][x],
                      &graph_info[y - 1][x], input_len, input_lines );
        testAddGraph( graph, y + 1, x, &q, &graph_info[y][x],
                      &graph_info[y + 1][x], input_len, input_lines );
        testAddGraph( graph, y, x - 1, &q, &graph_info[y][x],
                      &graph_info[y][x - 1], input_len, input_lines );
        testAddGraph( graph, y, x + 1, &q, &graph_info[y][x],
                      &graph_info[y][x + 1], input_len, input_lines );
    }
    if ( requirements != NULL ) {
        int *required =
            malloc( ( graph_info[target_y][target_x].required_count + 1 ) *
                    sizeof( size_t ) );
        for ( size_t i = 0; i < graph_info[target_y][target_x].required_count;
              i++ ) {
            for ( int j = 0; j < 26; j++ ) {
                if ( keys[j][0] ==
                         graph_info[target_y][target_x].required[i][0] &&
                     keys[j][1] ==
                         graph_info[target_y][target_x].required[i][1] ) {
                    required[i] = j;
                }
            }
        }
        required[graph_info[target_y][target_x].required_count] = -1;
        *requirements = required;
    }
    if ( pos[0] != target_x || pos[1] != target_y )
        return 0;
    return graph_info[target_y][target_x].path_len;
}

size_t keysToIndex( int found_keys[26] ) {
    size_t multiplier = 1;
    size_t index = 0;
    for ( int i = 0; i < 26; i++ ) {
        index += found_keys[i] * multiplier;
        multiplier *= 2;
    }
    return index;
}

void getOtherThree( int last_quadrants[4], int notincluded, int tmp[3] ) {
    int base = 0;
    for ( int i = 0; i < 3; i++ ) {
        if ( base + i == notincluded )
            base++;
        tmp[i] = last_quadrants[i + base];
    }
}

size_t shortestPathRec( size_t key_paths[26], size_t key_key_paths[26][26],
                          size_t path, int depth, int available_keys[26],
                          int found_keys[26], int *requirements[26],
                          int key_count, int prev[4], int keymap[26], size_t keys[26][3] ) {
    size_t local_best = -1;
    int next_prev[4];
    for ( int i = 0; i < key_count; i++ ) {
        if ( available_keys[i] == 1 ) {
            memcpy(next_prev, prev, 4 * sizeof(int));
            size_t new_path = path;
            found_keys[i] = 1;
            size_t glob_index = keysToIndex( found_keys );
            available_keys[i] = -1;
            for ( int j = 0; j < key_count; j++ ) {
                if ( available_keys[j] == 0 &&
                     dependenciesMet( requirements[j], found_keys ) )
                    available_keys[j] = 1;
            }
            // find best path to key
            size_t shortest_key_key = -1;
            int minpos = -1;
            for(int j = 0; j < 4; j++) {
                if(prev[j] != -1) {
                    if(key_key_paths[prev[j]][i] < shortest_key_key) {
                        shortest_key_key = key_key_paths[prev[j]][i];
                        minpos = j;
                        break;
                    }
                }
            }
            if(minpos == -1) {
                shortest_key_key = key_paths[i];
                minpos = keys[i][2];
            }
            new_path += shortest_key_key;
            next_prev[minpos] = i;
            if ( new_path >= local_best )
                goto cleanup;

            struct cache *cur_globCache = globCache[glob_index];

            if ( cur_globCache != 0 && getFromCache(cur_globCache, bestIndex(next_prev, keymap)) != 0 ) {
                found_keys[i] = 0;
                new_path += getFromCache(cur_globCache, bestIndex(next_prev, keymap));
                if ( new_path < local_best )
                    local_best = new_path;
                goto cleanup;
            }

            if ( depth < key_count ) {
                size_t result = shortestPathRec(
                    key_paths, key_key_paths, new_path, depth + 1,
                    available_keys, found_keys, requirements, key_count, next_prev, keymap, keys );
                if ( result < local_best )
                    local_best = result;
            }
        cleanup:
            found_keys[i] = 0;
            available_keys[i] = 1;
            for ( int j = 0; j < key_count; j++ ) {
                if ( available_keys[j] == 1 &&
                     !dependenciesMet( requirements[j], found_keys ) )
                    available_keys[j] = 0;
            }
            if ( depth == key_count ) {
                if ( new_path < local_best )
                    local_best = new_path;
            }
        }
    }
    struct cache *cur_globCache = globCache[keysToIndex(found_keys)];
    if(cur_globCache == 0) {
        globCache[keysToIndex(found_keys)] = calloc( 1, sizeof( struct cache ) );
        cur_globCache = globCache[keysToIndex(found_keys)];
        initCache(cur_globCache);
    }
    addToCache(cur_globCache, bestIndex(prev, keymap), local_best - path);
    return local_best;
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
        input_graph[input_lines] = malloc( input_len + 1 );
        memcpy( input_graph[input_lines], input, input_len );
        input_graph[input_lines][input_len] = '\0';
        input_lines++;
    }
    free( input );

    size_t start_x = 0, start_y = 0;
    int key_count = 0;
    size_t keys[26][3];
    size_t key_paths[26];
    size_t key_key_paths[26][26];
    int *key_requirements[26];
    struct helpingStruct **graph_info =
        malloc( input_lines * sizeof( struct helpingStruct * ) );
    for ( size_t i = 0; i < input_lines; i++ ) {
        graph_info[i] = calloc( input_len, sizeof( struct helpingStruct ) );
    }

    for ( size_t i = 0; i < input_lines; i++ ) {
        for ( size_t j = 0; j < input_len; j++ ) {
            if ( input_graph[i][j] == '@' ) {
                start_x = j;
                start_y = i;
            }
            if ( input_graph[i][j] >= 97 && input_graph[i][j] <= 122 ) {
                keys[key_count][0] = j;
                keys[key_count][1] = i;
                key_count++;
            }
        }
    }

    input_graph[start_y - 1][start_x] = '#';
    input_graph[start_y + 1][start_x] = '#';
    input_graph[start_y][start_x - 1] = '#';
    input_graph[start_y][start_x + 1] = '#';
    input_graph[start_y][start_x] = '#';

    int quadrants[4] = {0,0,0,0};
    int keymap[26] = {0};

    for ( int i = 0; i < key_count; i++ ) {
        keys[i][2] = 0;
        key_paths[i] = graphPath(
            input_graph, input_len, input_lines, start_x - 1, start_y - 1,
            keys[i][0], keys[i][1], graph_info, &key_requirements[i], keys );
        if ( key_paths[i] == 0 ) {
            free( key_requirements[i] );
            keys[i][2] = 1;
            key_paths[i] =
                graphPath( input_graph, input_len, input_lines, start_x + 1,
                           start_y - 1, keys[i][0], keys[i][1], graph_info,
                           &key_requirements[i], keys );
        }
        if ( key_paths[i] == 0 ) {
            free( key_requirements[i] );
            keys[i][2] = 2;
            key_paths[i] =
                graphPath( input_graph, input_len, input_lines, start_x - 1,
                           start_y + 1, keys[i][0], keys[i][1], graph_info,
                           &key_requirements[i], keys );
        }
        if ( key_paths[i] == 0 ) {
            free( key_requirements[i] );
            keys[i][2] = 3;
            key_paths[i] =
                graphPath( input_graph, input_len, input_lines, start_x + 1,
                           start_y + 1, keys[i][0], keys[i][1], graph_info,
                           &key_requirements[i], keys );
        }
        keymap[i] = quadrants[keys[i][2]];
        quadrants[keys[i][2]] += 1;

        resetGraph( graph_info, input_len, input_lines );
    }

    cacheBestMult[0] = MAXBEST / quadrants[0];
    for(int i = 1; i < 4; i++) {
        cacheBestMult[i] = cacheBestMult[i-1] / quadrants[i];
    }

    for ( int i = 0; i < 26; i++ ) {
        for ( int j = 0; j < 26; j++ ) {
            key_key_paths[i][j] = 0;
        }
    }

    for ( int i = 0; i < key_count; i++ ) {
        for ( int j = i + 1; j < key_count; j++ ) {
            if(keys[i][2] != keys[j][2]) {
                // if keys are in different quadrants, their
                // distance is "infinite"
                key_key_paths[i][j] = -1;
                key_key_paths[j][i] = key_key_paths[i][j];
            } else {
                key_key_paths[i][j] = graphPath(
                    input_graph, input_len, input_lines, keys[i][0], keys[i][1],
                    keys[j][0], keys[j][1], graph_info, NULL, NULL );
                key_key_paths[j][i] = key_key_paths[i][j];
                resetGraph( graph_info, input_len, input_lines );
            }
        }
    }
    for ( size_t i = 0; i < input_lines; i++ ) {
        free( input_graph[i] );
        free( graph_info[i] );
    }
    free( input_graph );
    free( graph_info );

    int available_keys[26];
    int found_keys[26];
    memset( available_keys, 0, 26 * sizeof( int ) );
    memset( found_keys, 0, 26 * sizeof( int ) );

    for ( int i = 0; i < 26; i++ ) {
        if ( dependenciesMet( key_requirements[i], found_keys ) )
            available_keys[i] = 1;
    }

    globCache = calloc( 67108864, sizeof( struct cache * ) );
    if ( globCache == NULL )
        error( EXIT_FAILURE, errno, "calloc" );
    int prev[4] = {-1,-1,-1,-1};
    printf( "MINIMUM PATH TO GET ALL KEYS: %zu\n",
            shortestPathRec( key_paths, key_key_paths, 0, 1, available_keys,
                             found_keys, key_requirements, key_count, prev, keymap, keys ) );
    for ( int i = 0; i < key_count; i++ ) {
        free( key_requirements[i] );
    }
    for(int i = 0; i < 67108864; i++) {
        if(globCache[i] != NULL) {
            free(globCache[i]->best);
            free(globCache[i]);
        }
    }
    free( globCache );
    fclose( in );
}
