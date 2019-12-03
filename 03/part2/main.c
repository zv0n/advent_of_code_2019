#define _GNU_SOURCE
#include <errno.h>
#include <error.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

int fillInput( int **input_in, char *path ) {
    int *input = *input_in;
    char *end = path;
    int line = 0;
    int column = 0;
    size_t input_size = 0;
    while ( *end != '\0' && *end != '\n' ) {
        int *tmp = realloc( input, ( input_size + 4 ) * sizeof( int ) );
        if ( tmp == NULL )
            error( EXIT_FAILURE, errno, "malloc" );
        input = tmp;
        input[input_size] = line;
        input[input_size + 1] = column;
        switch ( *path ) {
        case 'R':
            column += strtoul( path + 1, &end, 10 );
            break;
        case 'L':
            column -= strtoul( path + 1, &end, 10 );
            break;
        case 'U':
            line += strtoul( path + 1, &end, 10 );
            break;
        case 'D':
            line -= strtoul( path + 1, &end, 10 );
            break;
        }
        input[input_size + 2] = line;
        input[input_size + 3] = column;
        input_size += 4;
        path = end + 1;
    }
    *input_in = input;
    return input_size;
}

size_t lineLength( int start_l, int start_c, int end_l, int end_c ) {
    if ( start_l == end_l ) {
        if ( start_c < end_c )
            return end_c - start_c;
        return start_c - end_c;
    } else {
        if ( start_l < end_l )
            return end_l - start_l;
        return start_l - end_l;
    }
}

size_t pathLength( int *path, size_t finish_index ) {
    size_t length = 0;
    for ( size_t i = 0; i < finish_index; i += 4 ) {
        length += lineLength( path[i], path[i + 1], path[i + 2], path[i + 3] );
    }
    return length;
}

size_t crossesLines( int start, int end, int column, int *input,
                     size_t input_len, int *line_meet ) {
    size_t distance = -1;
    for ( size_t i = 0; i < input_len; i += 4 ) {
        if ( ( input[i + 1] <= column && input[i + 3] >= column ) ||
             ( input[i + 1] >= column && input[i + 3] <= column ) ) {
            if ( input[i] == input[i + 2] &&
                 ( ( start <= input[i] && end >= input[i] ) ||
                   ( start >= input[i] && end <= input[i] ) ) ) {
                size_t distance_local = pathLength( input, i );
                distance_local +=
                    lineLength( input[i], input[i + 1], input[i], column );
                if ( distance_local < distance ) {
                    distance = distance_local;
                    *line_meet = input[i];
                }
            }
        }
    }
    if ( distance == (size_t)-1 )
        return 0;
    return distance;
}

size_t crossesColumns( int start, int end, int line, int *input,
                       size_t input_len, int *column_meet ) {
    size_t distance = -1;
    for ( size_t i = 0; i < input_len; i += 4 ) {
        if ( ( input[i] <= line && input[i + 2] >= line ) ||
             ( input[i] >= line && input[i + 2] <= line ) ) {
            if ( input[i + 1] == input[i + 3] &&
                 ( ( start <= input[i + 1] && end >= input[i + 1] ) ||
                   ( start >= input[i + 1] && end <= input[i + 1] ) ) ) {
                size_t distance_local = pathLength( input, i );
                distance_local +=
                    lineLength( input[i], input[i + 1], line, input[i+1] );
                if ( distance_local < distance ) {
                    distance = distance_local;
                    *column_meet = input[i+1];
                }
            }
        }
    }
    if ( distance == (size_t)-1 )
        return 0;
    return distance;
}

size_t findCrossing( int *input, size_t input_len, char *path ) {
    char *end = path;
    int old_line = 0;
    int old_column = 0;
    int line = 0;
    int column = 0;
    size_t line_distance = 0;
    size_t min_distance = -1;
    while ( *end != '\0' && *end != '\n' ) {
        old_line = line;
        old_column = column;
        switch ( *path ) {
        case 'R':
            column += strtoul( path + 1, &end, 10 );
            break;
        case 'L':
            column -= strtoul( path + 1, &end, 10 );
            break;
        case 'U':
            line += strtoul( path + 1, &end, 10 );
            break;
        case 'D':
            line -= strtoul( path + 1, &end, 10 );
            break;
        }
        path = end + 1;
        size_t crossing = 0;
        int line_meet = 0;
        int column_meet = 0;
        size_t local_line_distance = line_distance;
        if ( column == old_column ) {
            crossing = crossesLines( old_line, line, column, input, input_len, &line_meet );
            local_line_distance += lineLength( old_line, old_column, line_meet, column );
        } else {
            crossing =
                crossesColumns( old_column, column, line, input, input_len, &column_meet );
            local_line_distance += lineLength( old_line, old_column, line, column_meet );
        }
        if ( crossing > 0 && crossing + local_line_distance < min_distance )
            min_distance = crossing + local_line_distance;
        line_distance += lineLength( old_line, old_column, line, column );
    }
    return min_distance;
}

int main() {
    FILE *in = fopen( "input", "r" );
    char *path1 = NULL;
    char *path2 = NULL;
    size_t input_len = 0;
    int *input1 = NULL;
    getline( &path1, &input_len, in );
    input_len = 0;
    getline( &path2, &input_len, in );
    input_len = fillInput( &input1, path1 );
    printf( "SHORTEST DISTANCE: %zu\n",
            findCrossing( input1, input_len, path2 ) );
    free( path1 );
    free( path2 );
}
