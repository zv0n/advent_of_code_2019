#ifndef DECK_H
#define DECK_H

#include <stdlib.h>

struct cardDeck {
    __int128 card_count;
    __int128 head;
    __int128 tail;
    __int128 cur_card;
    __int128 direction;
    __int128 skip;
    __int128 skip_index;
};

void initDeck( __int128 count, struct cardDeck *cd );
void dealIntoNewStack( struct cardDeck *cd );
void cutNCards( __int128 n, struct cardDeck *cd );
void dealWithIncrement( __int128 n, struct cardDeck *cd );
void printDeck( struct cardDeck *cd );
__int128 getValPos( struct cardDeck *cd, __int128 val );
__int128 iterativeGetValPos( struct cardDeck *cd, __int128 val, __int128 iteration );
__int128 atPos( struct cardDeck *cd, __int128 pos );
size_t sizeofDeck();

#endif
