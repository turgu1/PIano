#include "copyright.h"

#ifndef POLY_H
#define POLY_H

#include <sndfile.h>
#include "PIano.h"

#include "voice.h"
#include "fifo.h"

#define MAX_VOICES 128

#if USE_INTRINSICS
  #define FADE_OUT_FRAME_COUNT (20*1024)
#else
  #define FADE_OUT_FRAME_COUNT (40*1024)
#endif

class Poly : public NewHandlerSupport<Poly> {

 private:
  float        fadeOutScaleDown[FADE_OUT_FRAME_COUNT];
  voicep       voices;
  volatile int voiceCount;
  volatile int maxVoiceCount;
  buffp        tmpbuff;

  static void  outOfMemory();  ///< New operation handler when out of memory occurs

 public:
   Poly();
  ~Poly();

  int mixer(buffp buf, int frameCount);

  void   inactivateAllVoices();
  void   showState();
  voicep firstVoice();
  voicep nextVoice(voicep prev);
  void   addVoice(samplep s, char note, float gain);
  voicep nextAvailable();
  void   noteOff(char note, bool pedal_on);
  void   voicesSustainOff();
  voicep removeVoice(voicep v, voicep prev);
  int    getFrames(voicep v, buffp buff, int count);
  void   monitorCount();

  inline voicep getVoices() { return voices; }
  inline void   decVoiceCount() { voiceCount--; }
};

void * samplesFeeder(void * args);
void * sampleFileOpener(void * args);

#endif // POLY_H
