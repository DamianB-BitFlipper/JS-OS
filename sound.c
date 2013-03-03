
#include "sound.h"
#include "k_stdio.h"

extern globalFreq;

//Play sound using built in speaker
static void play_sound(u32int nFrequence)
{
 	u32int Div;
 	u8int tmp;

  //Set the PIT to the desired frequency
 	Div = 1193180 / nFrequence;
 	outb(0x43, 0xb6);
 	outb(0x42, (u8int) (Div) );
 	outb(0x42, (u8int) (Div >> 8));

  //And play the sound using the PC speaker
 	tmp = inb(0x61);
  if (tmp != (tmp | 3))
  {
    outb(0x61, tmp | 3);
  }
}

//make it shutup
static void nosound()
{
 	u8int tmp = (inb(0x61) & 0xFC);

 	outb(0x61, tmp);
 	//~ outb(0x61, ' ');
  
}

//Make a beep
void beep(int frequency, int milliseconds)
{
  play_sound(frequency);
 	mSleep(milliseconds);
 	nosound();
  //set_PIT_2(old_frequency);
}

void playNote(char *note, int milliseconds)
{
  int frequency;
  char firstLetter[1];

  destroyCharPointer(firstLetter);
  

  k_strchop(note, firstLetter, 0, 0);

  if(*note == 'C') //user wants to play a C
  {
    if(*(note + 1) == '4')
    {
      if(*(note + 2) == '#')
      {
        frequency = 277;
      }else{
        frequency = 262;
      }
    }else if(*(note + 1) == '5')
    {
      if(*(note + 2) == '#')
      {
        frequency = 554;
      }else{
        frequency = 523;
      }      
    }else if(*(note + 1) == '6')
    {
      if(*(note + 2) == '#')
      {
        frequency = 1109;
      }else{
        frequency = 1046;
      }      
    }

  }else if(*note == 'D') //user wants to play a D
  {
    if(*(note + 1) == '4')
    {
      if(*(note + 2) == 'b')
      {
        frequency = 277;
      }else if(*(note + 2) == '#')
      {
        frequency = 311;
      }else{
        frequency = 294;
      }      
    }else if(*(note + 1) == '5')
    {
      if(*(note + 2) == 'b')
      {
        frequency = 554;
      }else if(*(note + 2) == '#')
      {
        frequency = 622;
      }else{
        frequency = 587;
      }
    }
  }else if(*note == 'E') //user wants to play a D
  {
    if(*(note + 1) == '4')
    {
      if(*(note + 2) == 'b')
      {
        frequency = 311;
      }else{
        frequency = 330;
      }      
    }else if(*(note + 1) == '5')
    {
      if(*(note + 2) == 'b')
      {
        frequency = 622;
      }else
      {
        frequency = 659;
      }      
    }
  }else if(*note == 'F') //user wants to play a D
  {
    if(*(note + 1) == '4')
    {
      if(*(note + 2) == '#')
      {
        frequency = 370;
      }else{
        frequency = 349;
      }      
    }else if(*(note + 1) == '5')
    {
      if(*(note + 2) == '#')
      {
        frequency = 740;
      }else{
        frequency = 698;
      }      
    }
  }else if(*note == 'G') //user wants to play a D
  {
    if(*(note + 1) == '4')
    {
      if(*(note + 2) == 'b')
      {
        frequency = 370;
      }else if(*(note + 2) == '#')
      {
        frequency = 415;
      }else{
        frequency = 392;
      }      
    }else if(*(note + 1) == '5')
    {
      if(*(note + 2) == 'b')
      {
        frequency = 740;
      }else if(*(note + 2) == '#')
      {
        frequency = 831;
      }else{
        frequency = 784;
      }      
    }
  }else if(*note == 'A') //user wants to play a D
  {
    if(*(note + 1) == '4')
    {
      if(*(note + 2) == 'b')
      {
        frequency = 415;
      }else if(*(note + 2) == '#')
      {
        frequency = 466;
      }else{
        frequency = 440;
      }      
    }else if(*(note + 1) == '5')
    {
      if(*(note + 2) == 'b')
      {
        frequency = 831;
      }else if(*(note + 2) == '#')
      {
        frequency = 932;
      }else{
        frequency = 880;
      }      
    }
  }else if(*note == 'B') //user wants to play a D
  {
    if(*(note + 1) == '4')
    {
      if(*(note + 2) == 'b')
      {
        frequency = 466;
      }else{
        frequency = 494;
      }      
    }else if(*(note + 1) == '5')
    {
      if(*(note + 2) == 'b')
      {
        frequency = 932;
      }else{
        frequency = 988;
      }      
    }
  }

  //~ if(k_strcmp(note, "C4") == 0) //Middle C
  //~ {
    //~ frequency = 261;
  //~ }else if(k_strcmp(note, "D4") == 0) //Middle D
  //~ {
    //~ frequency = 293;
//~
  //~ }else if(k_strcmp(note, "E4") == 0) //Middle E
  //~ {
    //~ frequency = 329;
   //~
  //~ }else if(k_strcmp(note, "F4") == 0) //Middle F
  //~ {
    //~ frequency = 349;
    //~
  //~ }else if(k_strcmp(note, "G4") == 0) //Middle G
  //~ {
    //~ frequency = 392;
//~
  //~ }else if(k_strcmp(note, "A4") == 0) //Middle A
  //~ {
    //~ frequency = 440;
//~
  //~ }else if(k_strcmp(note, "B4") == 0) //Middle B
  //~ {
    //~ frequency = 493;
//~
  //~ }
  //~ }else if(k_strcmp(note, "C5") == 0) //Middle B
  //~ {
    //~ frequency = 493;
//~
  //~ }
  //~ }else if(k_strcmp(note, "D5") == 0) //Middle B
  //~ {
    //~ frequency = 493;
//~
  //~ }
  //~ }else if(k_strcmp(note, "E5") == 0) //Middle B
  //~ {
    //~ frequency = 493;
//~
  //~ }
  //~ }else if(k_strcmp(note, "F5") == 0) //Middle B
  //~ {
    //~ frequency = 493;
//~
  //~ }
  //~ }else if(k_strcmp(note, "G5") == 0) //Middle B
  //~ {
    //~ frequency = 493;
//~
  //~ }
  //~ }else if(k_strcmp(note, "A5") == 0) //Middle B
  //~ {
    //~ frequency = 493;
//~
  //~ }

  play_sound(frequency);
 	mSleep(milliseconds);
 	nosound();
}

void song_pacman()
{
  //~ asm volatile("sti");
  //~ init_timer(globalFreq); // Initialise timer to globalFreq-Hz
    
  int wholeNote = 1100;
  int half = wholeNote / 2, quarter = wholeNote / 4, eigth = wholeNote / 8, sixteenth = wholeNote / 16;

  //~ playNote("A5", eigth);
  playNote("B4", eigth);
  playNote("A5", eigth);
  playNote("F5#", eigth);
  playNote("E5b", eigth);

  playNote("A5", sixteenth);
  playNote("F5#", eigth);
  playNote("E5b", quarter);
  
  playNote("C5", eigth);
  playNote("C6", eigth);
  playNote("G5", eigth);
  playNote("E5", eigth);
  
  playNote("C6", sixteenth);
  playNote("G5", eigth);
  playNote("E5", quarter);
  
  playNote("B4", eigth);
  playNote("A5", eigth);
  playNote("F5#", eigth);
  playNote("E5b", eigth);

  playNote("A5", sixteenth);
  playNote("F5#", eigth);
  playNote("E5b", quarter);

  playNote("E5b", sixteenth);
  playNote("E5", sixteenth);
  playNote("F5", eigth);
  playNote("F5", sixteenth);
  playNote("F5#", sixteenth);
  playNote("G5", eigth);
  
  playNote("G5", sixteenth);
  playNote("A5b", sixteenth);
  playNote("A5", eigth);
  playNote("B5", quarter);
  

}
