#ifndef SOUND
#define SOUND

#include "common.h"

/*make the audio beep at frequency for milliseconds time*/
void beep(int frequency, int milliseconds);

/*Plays a note using a char array to type note*/
void playNote(char *note, int milliseconds);

/*Songs and tunes*/
//~ void song_pacman(int tempo);
void song_pacman(); //pacman tune

#endif
