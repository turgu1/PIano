#include "copyright.h"

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

#include "PIano.h"

void Config::init()
{
  //logger.DEBUG("init_config() ...\n");

  for (int i = 0; i < CFG_PARAMS_COUNT; i++) {
    switch (params[i].type) {
    case p_string:
      *((char **) params[i].param) = ((char *) params[i].default_value);
      break;
    case p_bool:
      *((bool *) params[i].param) = *((bool *) params[i].default_value);
      break;
    case p_int:
      *((int *) params[i].param) = *((int *) params[i].default_value);
      break;
    case p_float:
      *((float *) params[i].param) = *((float *) params[i].default_value);
      break;
    }
  }
}

void Config::showState()
{
  //logger.DEBUG("show_config() ...\n");

  for (int i = 0; i < CFG_PARAMS_COUNT; i++) {
    switch (params[i].type) {
    case p_string:
      fprintf(stdout, "%s = \"%s\"\n", params[i].name, *((char **) params[i].param));
      break;
    case p_bool:
      fprintf(stdout, "%s = %s\n", params[i].name, *((bool *) params[i].param) ? "true" : "false");
      break;
    case p_int:
      fprintf(stdout, "%s = %d\n", params[i].name, *((int *) params[i].param));
      break;
    case p_float:
      fprintf(stdout, "%s = %f\n", params[i].name, *((float *) params[i].param));
      break;

    }
  }
}

char Config::otoa(char c)
{
  if ((c >= '0') && (c <= '7')) return c - '0';
  return 0;
}

char Config::dtoa(char c)
{
  if ((c >= '0') && (c <= '9')) return c - '0';
  return 0;
}

char Config::xtoa(char c)
{
  if ((c >= '0') && (c <= '9')) return c - '0';
  if ((c >= 'a') && (c <= 'f')) return (c - 'a') + 10;
  if ((c >= 'A') && (c <= 'F')) return (c - 'A') + 10;
  return 0;
}

char Config::hextoa(char **p)
{
  char *s = *p;
  short c = 0;
  if (isxdigit(*s)) c = xtoa(*s++);
  if (isxdigit(*s)) c = (c << 4) + xtoa(*s++);
  *p = s;
  return (char) c;
}

char Config::octtoa(char **p)
{
  char *s = *p;
  short c = 0;
  if ((*s >= '0') && (*s <= '7')) c = otoa(*s++);
  if ((*s >= '0') && (*s <= '7')) c = (c << 3) + otoa(*s++);
  if ((*s >= '0') && (*s <= '7')) c = (c << 3) + otoa(*s++);
  *p = s;
  return (char) c;
}

int Config::dectoa(char **p)
{
  char *s = *p;
  int c = 0;
  if ((*s >= '0') && (*s <= '9')) c = dtoa(*s++);
  if ((*s >= '0') && (*s <= '9')) c = (c * 10) + dtoa(*s++);
  if ((*s >= '0') && (*s <= '9')) c = (c * 10) + dtoa(*s++);
  *p = s;
  return c;
}

//---- get_param ----
//
// return value:
//
// -1: some error append
//  0: success, param_name and param_value are set properly
//  1: success, don't care data present in line

int Config::get_param(char *buff, char **param_name, char **param_value, int line_nbr)
{
  //logger.DEBUG("get_param() ...\n");

  char * p = buff;

  while ((*p == ' ') || (*p == '\t')) p++;
  if ((*p == '#') || (*p == '\n') || (*p == '\r') || (*p == 0)) return 1;

  // param_name ...

  char *p1 = p;

  while (isalnum(*p1) || (*p1 == '_')) p1++;

  char *p2 = p1;

  // equal_sign ...

  while ((*p2 == ' ') || (*p2 == '\t')) p2++;
  if (*p2 != '=') {
    logger.ERROR("Syntax error in config file at line %d: '=' expected", line_nbr);
    return -1;
  }

  *p1 = 0;

  p2++;
  while ((*p2 == ' ') || (*p2 == '\t')) p2++;
  
  // param_value ...
  
  char *p3 = p2;

  if (*p3 == '"') {

    // string delimited with " ...

    p2++; p3++;
    char *p4 = p3;
    while ((*p3 != '"') && (*p3 != '\n') && (*p3 != '\r') && (*p3 != '\0')) {
      if (*p3 == '\\') {
	switch (*++p3) {
	case 't' : 
	case 'T' : *p4++ = '\t'; p3++; break; // tab
	case 'a' :
	case 'A' : *p4++ = '\a'; p3++; break; // bell
	case 'b' : 
	case 'B' : *p4++ = '\b'; p3++; break; // backspace
	case 'v' : 
	case 'V' : *p4++ = '\v'; p3++; break; // vertical tab
	case 'f' :
	case 'F' : *p4++ = '\f'; p3++; break; // form feed
	case 'r' : 
	case 'R' : *p4++ = '\r'; p3++; break; // return
	case 'n' : 
	case 'N' : *p4++ = '\n'; p3++; break; // new line
	case '\\': *p4++ = '\\'; p3++; break; // back slash
	case '\'': *p4++ = '\''; p3++; break; // quote
	case '"' : *p4++ = '"' ; p3++; break; // double quot
	case 'x' : 
	case 'X' : p3++; *p4++ = hextoa(&p3); break; // hex value
	case '0' : *p4++ = octtoa(&p3); break; // octal value
	case '1' :
	case '2' :
	case '3' :
	case '4' :
	case '5' :
	case '6' :
	case '7' :
	case '8' :
	case '9' : *p4++ = dectoa(&p3); break; // decimal value
	default: *p4++ = *p3++;
	}
      }
      else {
	*p4++ = *p3++;
      }
    } 
    *p4 = 0;
  }
  else if ((*p3 > ' ') && (*p3 <= '~')) {
    while ((*p3 > ' ') && (*p3 <= '~')) p3++;
    *p3 = 0;
  }
  else {
    logger.ERROR("Invalid parameter value in config file at line %d.", line_nbr);
    return -1;
  }

  *param_name = p;
  *param_value = p2;

  return 0;
}

Config::Config()
{
  FILE *f;

  f = fopen("~/.PIano.conf", "r");

  if (f != NULL) logger.INFO("Reading from ~/.PIano.conf...");
  else f = fopen("/etc/PIano.conf", "r");

  if (f != NULL) logger.INFO("Reading from /etc/PIano.conf...");
  else f = fopen("./PIano.conf", "r");

  if (f != NULL) logger.INFO("Reading from ./PIano.conf...");
  else logger.ERROR("No config file found.");

  init();

  if (f == NULL) logger.FATAL("Unable to find PIano.conf");

  int line_nbr = 0;
  int error_count = 0;

  while (!feof(f)) {

    char buff[1024];
    
    if (fgets(buff, 1023, f) == NULL) break;

    line_nbr++;

    char *param_name;
    char *value;
    int res = get_param(buff, &param_name, &value, line_nbr);

    if (res < 0) {
      error_count++;
    }
    else if (res == 0) {
      bool found = false;
      char *s = param_name;

      while (*s) {
	*s = tolower(*s);
	s++;
      }

      for (int i = 0; i < CFG_PARAMS_COUNT; i++) {
	if (strcmp(params[i].name, param_name) == 0) {
          char * b;

	  found = true;
          switch (params[i].type) {
          case p_string:
	    b = (char *) malloc(strlen(value) + 1); // TODO: Memory leak
	    strcpy(b, value);
	    *((char **) params[i].param) = b;
            break;
          case p_int:
	    *((int *) params[i].param) = atoi(value);
            break;
          case p_float:
	    *((float *) params[i].param) = atof(value);
            break;
          case p_bool:
	    if (strcmp(value, "false") == 0) {
	      *((bool *) params[i].param) = false;
	    }
	    else if (strcmp(value, "true") == 0) {
	      *((bool *) params[i].param) = true;
	    }
	    else {
	      logger.ERROR("Expected boolean value true or false at config file line %d.", line_nbr);
	      error_count++;
	    }
            break;
          default:
            logger.FATAL("Internal error reading configuration file.");
          }
	}
      }
      if (!found) {
	logger.ERROR("Unknown parameter: %s.", param_name);
	error_count++;
      }
    }
  }

  fclose(f);

  if (error_count > 0) logger.FATAL("Errors found in config file: %d.", error_count);
}
