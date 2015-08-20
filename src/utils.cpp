#include "copyright.h"

#include "PIano.h"

// Note pitch Midi equivalencies
//
// c c#  d d#  e  f f#  g g#  a a#  b
// 0  1  2  3  4  5  6  7  8  9 10 11
//
// note = <octave# + 1> * 12 + <note value>
//
// middle c = c4 = 60 = 5 * 12 + 0


PRIVATE const char * notesStr[12]   = { "c", "c#", "d", "d#", "e", "f", "f#", "g", "g#", "a", "a#", "b" };
PRIVATE const char * octavesStr[10] = { "0", "1", "2", "3", "4", "5", "6", "7", "8", "9" };
PRIVATE const char   valuesStr[7]   = { 9, 11, 0, 2, 4, 5, 7 };

//---- noteToValue() ----

char noteToValue(char * note)
{
  char c = tolower(*note++);
  char val;

  if ((c < 'a') || (c > 'g')) return 0;
  val = valuesStr[c - 'a'];
  if (*note == '#') {
    val++;
    note++;
  }
  if ((*note < '0') || (*note > '9')) return 0;

  return ((*note - '0') + 1) * 12 + val;
}

//---- valueToNote() ----

char * valueToNote(char value)
{
  static char   noteStr[5];
  short note, octave;
  
  octave = (value / 12) - 1;
  if (octave < 0) octave = 0;

  note = value % 12;
  strcpy(noteStr, notesStr[note]);
  strcat(noteStr, octavesStr[octave]);

  return noteStr;  
}

