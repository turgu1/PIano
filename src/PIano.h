#include "copyright.h"

// This file contains all globally identified data for the PIano application.

#ifndef PIANO_H
#define PIANO_H

#define PRIVATE static

#define USE_INTRINSICS 1

#define SUCCESS 0

#define MIN(a, b) (a < b) ? a : b
#define MAX(a, b) (a > b) ? a : b

typedef float sample_t;
typedef sample_t * buffp;

// Number of parameters in the cfg structure below

#define CFG_PARAMS_COUNT 14

#define BUFFER_FRAME_COUNT  256
#define FRAME_SIZE          (2 * sizeof(sample_t))
#define LOG_FRAME_SIZE      3
#define FRAME_BUFFER_SIZE   (BUFFER_FRAME_COUNT * FRAME_SIZE)
#define BUFFER_SAMPLE_COUNT (BUFFER_FRAME_COUNT * 2)

#include <iostream>
#include <cstdlib>
#include <cstring>
#include <cassert>
#include <cstdarg>
#include <new>

#ifdef PIANO
# define PUBLIC
#else
# define PUBLIC extern
#endif

#include "new_handler_support.h"

#include "log.h"

PUBLIC Log logger;

#include "config.h"
#include "utils.h"

#include "sample.h"
#include "samples_library.h"
#include "sound.h"
#include "midi.h"
#include "reverb.h"
#include "poly.h"
#include "equalizer.h"

PUBLIC struct configStruct {
  char  * samplesLibrariesLocation;
  int     masterVolume;
  bool    enableReplay;
  struct pcmStruct {
    int     deviceNbr;
    char  * deviceName;
  } pcm;
  struct midiStruct {
    int     sustainTreshold;
    int     deviceNbr;
    char  * deviceName;
    int     channel;
  } midi;
  struct reverbScruct {
    float   roomSize;
    float   damping;
    float   width;
    float   dryWet;
    float   apGain;
  } reverb;
} cfg;

PUBLIC double   samplingRate;       ///< Sampling Rate. Will be initialized to 44100. Not supposed to be modified at thispoint in time.
PUBLIC float    masterVolume;       ///< Overall master volume (gain)
PUBLIC bool     interactive;        ///< True if PIano launched in interactive mode (option -i)
PUBLIC bool     silent;             ///< If true, no logging message will be displayed
PUBLIC bool     replayEnabled;      ///< Enable replaying last 5 seconds played
PUBLIC volatile bool keepRunning;   ///< True while the application is running

// Statistics

PUBLIC int      maxVoicesMixed;     ///< Maximum number of voices that have been mixed simultaneously
PUBLIC long     mixerDuration;      ///< Maximum duration of the mixer function during play (nanoseconds)
PUBLIC long     reverbMinDuration;  ///< Minimum duration of the reverb process
PUBLIC long     reverbMaxDuration;  ///< Maximim duration of the reverb process
PUBLIC sample_t maxVolume;          ///< Maximum gain used un mixing voices

// Global Application objects

PUBLIC Config         * config;
PUBLIC SamplesLibrary * samples;
PUBLIC Sound          * sound;
PUBLIC Midi           * midi;
PUBLIC Reverb         * reverb;
PUBLIC Poly           * poly;
PUBLIC Equalizer      * equalizer;

/// Class PIano is to outer layer class of the application. Its main objective is to
/// ensure proper sequencing in the initialization and rundown process for all global
/// objects that are part of PIano.

class PIano : public NewHandlerSupport<PIano> {

 private:
  static void outOfMemory();  ///< New operation handler when out of memory occurs

 public:
   PIano();
  ~PIano();
};

PUBLIC PIano * piano;

#undef PUBLIC
#endif
