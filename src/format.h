#include "copyright.h"

#ifndef FORMAT_H
#define FORMAT_H

#ifdef FORMAT
# define PUBLIC
#else
# define PUBLIC extern
#endif

#ifdef FORMAT

  typedef struct format_struct *formatp;

  struct format_struct {
    formatp next;
    char * descriptor;
  };

  PRIVATE formatp formats = NULL;

  PRIVATE char note_values_str[7] = { 9, 11, 0, 2, 4, 5, 7 };

  PRIVATE bool note_found;
  PRIVATE bool volume_found;
  PRIVATE char the_note;
  PRIVATE char the_volume;

  PRIVATE char *   midi_volume_command(char * fn, char * params[], int param_cnt);
  PRIVATE char *    dec_volume_command(char * fn, char * params[], int param_cnt);
  PRIVATE char *    sfz_volume_command(char * fn, char * params[], int param_cnt);
  PRIVATE char * custom_volume_command(char * fn, char * params[], int param_cnt);
  PRIVATE char *   offset_note_command(char * fn, char * params[], int param_cnt);
  PRIVATE char *     midi_note_command(char * fn, char * params[], int param_cnt);
  PRIVATE char *          note_command(char * fn, char * params[], int param_cnt);

  #define COMMAND_COUNT 7

  PRIVATE struct command_struct {
    const char * name;
    char * (* function)(char * fn, char * params[], int param_cnt);
  } commands[COMMAND_COUNT] = {
    { "note",          note_command          },
    { "midi_note",     midi_note_command     },
    { "offset_note",   offset_note_command   },
    { "dec_volume",    dec_volume_command    },
    { "sfz_volume",    sfz_volume_command    },
    { "custom_volume", custom_volume_command },
    { "midi_volume",   midi_volume_command   }
  };

#endif

PUBLIC int parse_filename(char * filename, char * note, char * volume);
PUBLIC int load_format_file(char * location);

#undef PUBLIC
#endif
