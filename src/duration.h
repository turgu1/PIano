#ifndef DURATION_H
#define DURATION_H

#include <time.h>

class Duration {
 private:
  struct timespec starttime;

 public:
  Duration();
  long getElapse();
};

#endif
