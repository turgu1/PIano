#include "copyright.h"

#ifndef CONFIG_H
#define CONFIG_H

typedef enum { p_string, p_bool, p_int, p_float } param_type;

typedef struct {
  const char * name;
  void       * param;
  param_type   type;
  const void * default_value;
} paramStruct;

class Config {

 private:

  /// This is the configuration parameters structure that contains the information for all accpetable config
  /// elements of PIano. The initialization of this structure is done statically inside main.cpp source file.
  static paramStruct params[CFG_PARAMS_COUNT];
  
  char otoa(char c);
  char dtoa(char c);
  char xtoa(char c);

  char hextoa(char **p);
  char octtoa(char **p);
  int  dectoa(char **p);

  void init();

 public:
  Config();

  int get_param(char *buff, char **param_name, char **param_value, int line_nbr);
  void showState();
};

#endif
