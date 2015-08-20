#include "copyright.h"

#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#include "PIano.h"

#define FORMAT
#include "format.h"

PRIVATE void clear_formats()
{
  formatp fmt = formats;
  formatp nxt;

  while (fmt) {
    nxt = fmt->next;
    free(fmt->descriptor);
    free(fmt);
    fmt = nxt;
  }
  formats = NULL;
}

PRIVATE char * extract_str(char * param)
{
  static char p[256];

  int i = 0;
  while ((i < 255) && param[i] && (strchr(":}", param[i]) == NULL)) {
    p[i] = param[i];
    i++;
  }
  p[i] = '\0';

  return p;
}

PRIVATE char * midi_volume_command(char * fn, char * params[], int param_cnt)
{
  (void) params;
  (void) param_cnt;

  short volume = 0;

  while (isdigit(*fn)) volume = (volume * 10) + (*fn++ - '0');
  if ((volume < 0) || (volume > 127)) return NULL;

  the_volume = volume;
  volume_found = true;

  return fn;
}

// ppp: 16    Piano Pianissimo
// pp: 32     Pianissimo
// p: 48      Piano
// mp: 64     Mezzo-piano
// mf: 80     Mezzo-forte
// f: 96      forte
// ff: 112    fortissimo
// fff: 127   forte fortissimo

// ppp: 8    Piano Pianissimo
// pp: 24     Pianissimo
// p: 40      Piano
// mp: 56     Mezzo-piano
// mf: 72     Mezzo-forte
// f: 88      forte
// ff: 104    fortissimo
// fff: 127   forte fortissimo

PRIVATE char *  sfz_volume_command(char * fn, char * params[], int param_cnt)
{
  (void) params;
  (void) param_cnt;

  short volume;

  switch (tolower(*fn++)) {
  case 'p':
    switch (tolower(*fn++)) {
    case 'p':
      if (tolower(*fn) == 'p') {
	volume = 16;
	fn++;
      }
      else {
	volume = 24;
      }
      break;
    default:
      volume = 40;
      fn--;
      break;
    }
    break;
  case 'm':
    switch (tolower(*fn++)) {
    case 'p':
      volume = 56;
      break;
    case 'f':
      volume = 72;
      break;
    default:
      return NULL;
    }
    break;
  case 'f':
    switch (tolower(*fn++)) {
    case 'f':
      if (tolower(*fn) == 'f') {
	volume = 127;
	fn++;
      }
      else {
	volume = 104;
      }
      break;
    default:
      volume = 88;
      fn--;
      break;
    }
    break;
  default:
    return NULL;
  }

  volume_found = true;
  the_volume = volume;

  return fn;
}

PRIVATE char * check_str_param(char * fn, char * param)
{
  char *p = strchr(param, '=');
  
  if (p == NULL) {
    logger.ERROR("Badly formated parameter: %s\n", param);
    return NULL;
  }
  if (strncmp(fn, param, p - param) != 0) return NULL;

  fn += (p - param);

  int volume = 0;
  p++;

  while (isdigit(*p)) volume = (volume * 10) + (*p++ - '0');
  the_volume = volume;
  volume_found = true;

  return fn;
}

PRIVATE char * custom_volume_command(char * fn, char * params[], int param_cnt)
{
  char * pos;
  for (int i = 0; i < param_cnt; i++) {
    if ((pos = check_str_param(fn, params[i])) != NULL) {
      return pos;
    }
  }
  return NULL;
}

PRIVATE char * dec_volume_command(char * fn, char * params[], int param_cnt)
{
  if ((param_cnt != 2) && (param_cnt != 4)) {
    logger.ERROR("Format Descriptor: 2 or 4 parameters expected for dec_volume command\n");
    return NULL;
  }

  short low = atoi(extract_str(params[0]));
  short high = atoi(extract_str(params[1]));
  short midi_high = 0;
  short midi_low = 127;

  if (param_cnt == 4) {
    midi_low = atoi(extract_str(params[2]));
    midi_high = atoi(extract_str(params[3]));
  }

  //logger.DEBUG("Low:%d, High:%d", low, high);

  if (low >= high) {
    logger.ERROR("Format Descriptor: Low parameter must be < High parameter for volume command");
    return NULL;
  }

  long volume = 0;
  while (isdigit(*fn)) volume = (volume * 10) + (*fn++ - '0');
  if ((volume < low) || (volume > high)) return NULL;

  if (param_cnt == 2) {
    the_volume = (volume * 127) / (high - low + 1);
  }
  else {
    the_volume = ((volume * (midi_high - midi_low)) / (high - low + 1)) + midi_low;
  }
  volume_found = true;

  //logger.DEBUG("Volume: %d", the_volume);
  return fn;
}


PRIVATE char * offset_note_command(char * fn, char * params[], int param_cnt)
{
  if (param_cnt != 1) {
    logger.ERROR("Format Descriptor: 1 parameter expected for offset_note command\n");
    return NULL;
  }

  short offset = atoi(extract_str(params[0]));

  short note = 0;
  while (isdigit(*fn)) note = (note * 10) + (*fn++ - '0');

  note += offset;
  if ((note < 0) || (note > 127)) return NULL;

  the_note = note;
  note_found = true;

  return fn;
}

PRIVATE char * midi_note_command(char * fn, char * params[], int param_cnt)
{
  (void) params;
  (void) param_cnt;

  short note = 0;

  while (isdigit(*fn)) note = (note * 10) + (*fn++ - '0');
  if ((note < 0) || (note > 127)) return NULL;

  the_note = note;
  note_found = true;

  return fn;
}

PRIVATE char * note_command(char * fn, char * params[], int param_cnt)
{
  (void) params;
  (void) param_cnt;

  int note_idx = tolower(*fn++) - 'a';

  if ((note_idx >= 0) && (note_idx < 7)) {
    short note = note_values_str[note_idx];
    short offset = 0;
    if (*fn == '#') {
      offset = 1;
      fn++;
    }
    else if (*fn == 'b') {
      offset = -1;
      fn++;
    }
    short octave = *fn++ - '0';
    if ((octave < 0) || (octave > 9)) return NULL;

    the_note = (octave + 1) * 12 + note + offset;
    note_found = true;

    //logger.DEBUG("Note: %d", the_note);
    return fn;
  }
  return NULL;
}

PRIVATE char * exec_command(char * fn, char * cmd, char * params[], int param_cnt)
{
  char * cmds = extract_str(cmd);
  for (int i = 0; i < COMMAND_COUNT; i++) {
    if (strcmp(cmds, commands[i].name) == 0) {
      return commands[i].function(fn, params, param_cnt);
    }
  }
  logger.ERROR("Format Descriptor: Unknown command: %s.", cmds);

  return NULL;
}

PRIVATE void show_descriptors()
{
  formatp fmt = formats;

  logger.DEBUG("[File Format Descriptor List]");
  while (fmt != NULL) {
    logger.DEBUG("%s", fmt->descriptor);
    fmt = fmt->next;
  }
  logger.DEBUG("[End]");
}

PRIVATE void show_command(char * cmd, char ** params, int param_cnt)
{
  logger.DEBUG("Descriptor: %s (%d params)", extract_str(cmd), param_cnt);
  for (int i = 0; i < param_cnt; i++) {
    logger.DEBUG("  Param %d: %s", i+1, extract_str(params[i]));
  }
}
 
int parse_filename(char * filename, char * note, char * volume)
{
  formatp fmt = formats;
  
  note_found = false;
  volume_found = false;

  while (fmt != NULL) {

    char * descrip = fmt->descriptor;
    char * fn = filename;
    char * p;
    char * p2;
    char * p3; 
    char * p4;
    char * params[20];
    int pidx = 0;

    //logger.DEBUG("while fmt...\n");

    while (true) {
    next:
      while (strchr("*+?{", *descrip) == NULL) {
	if (*descrip++ != *fn++) goto cont;
      }
      switch (*descrip) {
      case '\0':
	if ((*fn == '\0') && note_found && volume_found) {
          *note = the_note;
          *volume = the_volume;
          return SUCCESS;
        }
        goto cont;
      case '+':  // 1 or more characters
	fn++;
      case '*':  // 0 or more characters
	p = ++descrip;
        if (*p == '\0') goto next;
        while (strchr("*+?{", *p) == NULL) {
	  // Pass to the point that seems to be after '*' or '+'
	  while (*fn && (*p != *fn)) fn++;
	  if (*fn == '\0') {
	    if (*p == '\0') {
	      descrip = p;
	      goto next;
	    }
	    goto cont;
	  }  
	  // Check if the following chars in filename match the pattern after '*' or '+'
	  while (*p == *fn) {
	    p++; fn++;
	    if (strchr("*+?{", *p)) {
	      descrip = p;
	      goto next;
	    }
	    if (*fn == '\0') goto cont;
	  }
	  p = descrip;
	}
	break;
      case '?':  // any 1 character
	descrip++;
	fn++;
	break;
      case '{':
	p = p2 = ++descrip;
	while ((*p2 != '\0') && (*p2 != ':') && (*p2 != '}')) p2++;
	if (*p2 == '\0') {
	  logger.ERROR("Format Descriptor: Expecting closing brace:\n%s", fmt->descriptor);
	  return -1;
	}
	if (*p2 == ':') {
	  p3 = p4 = ++p2;
	  pidx = 0;
	  while (*p4 && (*p4 != '}')) {
	    if (pidx >= 20) {
	      logger.ERROR("Format Descriptor: Too many command parameters:\n%s", 
			   fmt->descriptor);
	      return -1;
	    }
	    params[pidx++] = p3;
	    while (*p4 && (*p4 != '}') && (*p4 != ':')) p4++;
	    if (*p4 == ':') p4++;
	    p3 = p4;
	  }
	  if (*p4 != '}') {
	    logger.ERROR("Format Descriptor: Expecting closing brace:\n%s", 
			 fmt->descriptor);
	    return -1;
          }
          descrip = ++p4;
	}
	else if (*p2 != '}') {
	  logger.ERROR("Format Descriptor: Expecting closing brace:\n%s", 
		       fmt->descriptor);
	  return -1;
	}
        else {
          descrip = p2 + 1;
        }
        //show_command(p, params, pidx);
        if ((fn = exec_command(fn, p, params, pidx)) == NULL) goto cont;
	break;
      default:
	logger.FATAL("Format Descriptor: Internal error in function parse_filename()!\nDescriptor: %s\nFilename: %s",
		     fmt->descriptor,
		     filename);
	break;
      }
    }

  cont:

    fmt = fmt->next;
  }

  return -1; // No format match filename
}

PRIVATE const char * default_descriptors[3] = {
  "{note}v{dec_volume:1:16}.flac",
  "{note}v{dec_volume:1:16}.wav",
  NULL
};

int load_format_file(char * location)
{
  //logger.DEBUG("load_format_file()\n");

  clear_formats();

  char buff[1024];

  strcpy(buff, location);
  strcat(buff, "/");
  strcat(buff, "format.txt");

  FILE *f = fopen(buff, "r");

  formatp fmt;
  formatp prev = NULL;

  if (f == NULL) {
    logger.DEBUG("Unable to open file %s.  Using default file descriptors.", buff);
    const char ** des = default_descriptors;
    while (*des) {
      fmt = (formatp) malloc(sizeof(struct format_struct));
      fmt->descriptor = (char *) malloc(strlen(*des) + 1);
      strcpy(fmt->descriptor, *des);
      fmt->next = NULL;
      if (prev == NULL) {
        formats = fmt;
      }
      else {
        prev->next = fmt;
      }
      prev = fmt;
      des++;
    }
    return SUCCESS;
  }

  formats = NULL;

  while (!feof(f)) {

    if (fgets(buff, 1023, f) == NULL) break;

    if (buff[strlen(buff) - 1] == '\n') buff[strlen(buff) - 1] = 0;
    if (buff[strlen(buff) - 1] == '\r') buff[strlen(buff) - 1] = 0;
    if (buff[strlen(buff) - 1] == '\n') buff[strlen(buff) - 1] = 0;

    char * s = buff;

    while ((*s == '\t') || (*s == ' ')) s++;
    if ((*s == '\0') || (*s == '#') || (*s == '\n') || (*s == '\r')) continue;

    fmt = (formatp) malloc(sizeof(struct format_struct));
    fmt->next = NULL;
    fmt->descriptor = (char *) malloc(strlen(s) + 1);
    strcpy(fmt->descriptor, s);

    if (prev == NULL) {
      formats = fmt;
    }
    else {
      prev->next = fmt;
    }
    prev = fmt;
  }

  //show_descriptors();
  return SUCCESS;
}
