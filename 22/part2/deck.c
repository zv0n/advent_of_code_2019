#include "deck.h"

#include <errno.h>
#include <error.h>
#include <limits.h>
#include <stdio.h>

__int128 modulo( __int128 x, __int128 n ) {
    if( x < 0 ) {
        x %= n;
        x += n;
    } else {
        x %= n;
    }
    return x;
}

__int128 powMod( __int128 a, __int128 b, __int128 mod ) {
    __int128 ret = a;
    __int128 power = 1;
    __int128 *powers = malloc( sizeof( __int128 ) * 100000 );
    powers[0] = a;
    __int128 index = 0;
    while( power < b/2 ) {
        ret *= ret;
        power *= 2;
        index++;
        ret = modulo( ret, mod );
        if( index == 100000 )
            break;
        powers[index] = ret;
    }
    __int128 working_power = power/2;
    for( __int128 i = index - 1; i > 0 ; i-- ) {
        while( power + working_power < b ) {
            power += working_power;
            ret *= powers[i];
            ret = modulo( ret, mod );
        }
        working_power /= 2;
    }
    for( __int128 i = power; i < b; i++ ) {
        ret *= a;
        ret = modulo( ret, mod );
    }
    ret = modulo( ret, mod );
    return ret;
}

__int128 nextCard( struct cardDeck *cd ) {
    __int128 ret = cd->cur_card;
    cd->cur_card += cd->skip * cd->direction;
    cd->cur_card = modulo( cd->cur_card, cd->card_count );
    return ret;
}

void resetPosition( struct cardDeck *cd ) {
    cd->cur_card = cd->head;
}

void initDeck( __int128 count, struct cardDeck *cd ) {
//    printf( "INITIALIZING\n" );
    cd->head = 0;
    cd->tail = count - 1;
    cd->direction = 1;
    cd->skip = 1;
    cd->skip_index = 1;
    cd->card_count = count;
    resetPosition( cd );
}

void dealIntoNewStack( struct cardDeck *cd ) {
//    printf( "NEW StACKING\n" );
    cd->direction *= -1;
    __int128 backup = cd->head;
    cd->head = cd->tail;
    cd->tail = backup;
    resetPosition( cd );
}

void cutNCards( __int128 n, struct cardDeck *cd ) {
//    printf( "CUTTING\n" );
    cd->head += n * cd->skip * cd->direction;
    cd->tail += n * cd->skip * cd->direction;
    cd->head = modulo( cd->head, cd->card_count );
    cd->tail = modulo( cd->tail, cd->card_count );
    resetPosition( cd );
}

void dealWithIncrement( __int128 n, struct cardDeck *cd ) {
//    printf( "DEALING WITH INCREMENT\n" );
    cd->skip_index *= n;
    cd->skip_index = modulo( cd->skip_index, cd->card_count );
    __int128 i = 0;
    while( ( 1 + cd->card_count * i ) % n != 0 )
        i++;
    cd->skip *= (1 + cd->card_count * i) / n;
    cd->skip = modulo( cd->skip, cd->card_count );
    cd->tail = cd->head + cd->direction * (cd->card_count - 1) * cd->skip;
    cd->tail = modulo( cd->tail, cd->card_count );
}

void printDeck( struct cardDeck *cd ) {
    for( __int128 i = 0; i < cd->card_count; i++ ) {
        printf( "%li ", nextCard( cd ) );
    }
    printf( "\n" );
}

__int128 getValPos( struct cardDeck *cd, __int128 val ) {
    __int128 valindex = cd->direction * ( val - cd->head );
    valindex = modulo( valindex, cd->card_count );
    __int128 a = valindex * cd->skip_index;
    a %= cd->card_count;
    return a;
}

__int128 iterativeGetValPos( struct cardDeck *cd, __int128 val, __int128 iteration ) {
    __int128 p = powMod( cd->direction * cd->skip_index, iteration, cd->card_count );
    __int128 rest = powMod( 1 - cd->direction * cd->skip_index, cd->card_count - 2, cd->card_count );
    rest = modulo( (1-p) * rest, cd->card_count );
    // we don't want power of 0
    rest--;
    return modulo( p * val - p * cd->head - rest * cd->head, cd->card_count );
}

__int128 atPos( struct cardDeck *cd, __int128 pos ) {
    __int128 local_skip = cd->head +  pos * cd->skip * cd->direction;
    local_skip %= cd->card_count;
    return modulo( local_skip, cd->card_count );
}

size_t sizeofDeck() {
    return sizeof( struct cardDeck );
}
