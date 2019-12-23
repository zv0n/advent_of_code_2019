#ifndef DECK_H
#define DECK_H

#include <stdlib.h>

struct cardDeck;

void initDeck( __int128 count, struct cardDeck *cd );
void dealIntoNewStack( struct cardDeck *cd );
void cutNCards( __int128 n, struct cardDeck *cd );
void dealWithIncrement( __int128 n, struct cardDeck *cd );
void printDeck( struct cardDeck *cd );
__int128 getValPos( struct cardDeck *cd, __int128 val );
__int128 iterativeGetValPos( struct cardDeck *cd, __int128 val, __int128 iteration );
__int128 atPos( struct cardDeck *cd, __int128 pos );
__int128 iterativeAtPos( struct cardDeck *cd, __int128 pos, __int128 iteration );
size_t sizeofDeck();

#endif
