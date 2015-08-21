#include "copyright.h"

#include <cstring>
#include <cstdlib>
#include <sndfile.h>

#include "PIano.h"
#include "sample.h"

#define FRAMES_IN_CACHE (1024 * 40)

//---- Sample() ----

Sample::Sample(char * fname, char note, int volume)
{
  setNewHandler(outOfMemory);

  filename = new char[strlen(fname) + 1];

  strcpy(filename, fname);
  this->note   = note;
  this->volume = volume;
  next   = NULL;

  SF_INFO sfinfo;

  sfinfo.format = 0;

  SNDFILE * f = sf_open(filename, SFM_READ, &sfinfo);
  if (f == NULL) {
    logger.FATAL("Unable to open sound file %s: %s.", filename, sf_strerror(NULL));
  }

  // Load the first FRAMES_IN_CACHE frames  (close to 1 second of data)
  // Better be on a pcm sound device buffer size frontier to simplify 
  // transition between the samples cache content and the fifo managed 
  // in the voice structure.

  buff = new sample_t[FRAMES_IN_CACHE * 2];

  frameCount = sf_readf_float(f, buff, FRAMES_IN_CACHE);

  sf_close(f);
}

//---- ~Sample() ----

Sample::~Sample()
{
  delete [] filename;
  delete [] buff;
}

//----- outOfMemory() ----

void Sample::outOfMemory()
{
  logger.FATAL("Sample: Unable to allocate memory.");
}

//---- showFrames() ----

void Sample::showFrames(int idx, int frameCount)
{
  int i = 0;

  using namespace std;

  logger.DEBUG("FRAMES DUMP:");
  while (i++ < frameCount) {
    cerr << "[" << buff[idx] << ", " << buff[idx + 1] << "] ";
    idx += 2;
    if ((i % 8) == 0) cerr << endl;
  }
  logger.DEBUG("\nEND\n");
}

//---- showState() -----

void Sample::showState()
{
  using namespace std;

  cout << "sample: "
       << "note:"  << note       << " "
       << "vol:"   << volume     << " "
       << "buff:"  << buff       << " "
       << "fc:"    << frameCount << " "
       << "nxt:"   << next       << " "
       << "fname:" << filename   << endl;
}
