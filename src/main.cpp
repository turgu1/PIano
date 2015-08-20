#include "copyright.h"

#include <csignal>
#include <pthread.h>

#define PIANO
#include "PIano.h"

#include "interactive_mode.h"

// Configuration parameter table initialization

static int     defaultVolume      =    50;
static float   defaultRoomSize    = 0.84f;
static float   defaultDamping     = 0.20f;
static float   defaultWidth       = 0.40f;
static float   defaultDryWet      = 0.75f;
static float   defaultApGain      = 0.50f;
static int     defaultDeviceNbr   =    -1;
static int     defaultChannel     =    -1;
static bool    defaultFalse       = false;

paramStruct Config::params[CFG_PARAMS_COUNT] = {
  { "samples_libraries_location", &cfg.samplesLibrariesLocation, p_string, "." },
  { "master_volume",    &cfg.masterVolume,     p_int,    &defaultVolume        },
  { "enable_replay",    &cfg.enableReplay,     p_bool,   &defaultFalse         },
  { "pcm_device_nbr",   &cfg.pcm.deviceNbr,    p_int,    &defaultDeviceNbr     },
  { "pcm_device_name",  &cfg.pcm.deviceName,   p_string, "DAC"                 },
  { "midi_channel",     &cfg.midi.channel,     p_int,    &defaultChannel       },
  { "midi_device_nbr",  &cfg.midi.deviceNbr,   p_int,    &defaultDeviceNbr     },
  { "midi_device_name", &cfg.midi.deviceName,  p_string, "Roland"              },
  { "reverb_room_size", &cfg.reverb.roomSize,  p_float,  &defaultRoomSize      },
  { "reverb_damping",   &cfg.reverb.damping,   p_float,  &defaultDamping       },
  { "reverb_width",     &cfg.reverb.width,     p_float,  &defaultWidth         },
  { "reverb_dry_wet",   &cfg.reverb.dryWet,    p_float,  &defaultDryWet        },
  { "reverb_ap_gain",   &cfg.reverb.apGain,    p_float,  &defaultApGain        }
};

//---- sigint_handler ----

/// The sigintHanfler() function receive control when a signal is sent to the application. The
/// current behaviour is to start a rundown of PIano, setting the keepRunning boolean value
/// to false.
void sigintHandler(int dummy)
{
  (void) dummy;
  keepRunning = false;
}

//---- PIano() ----

/// Initialization method for the PIano application.
PIano::PIano()
{
  setNewHandler(outOfMemory);

  logger.INFO("Initializing...");

  keepRunning = true;

  config = new Config();

  samplingRate       = 44100.0f;  // Not sure this can be changed without consequences

  masterVolume       = cfg.masterVolume / 100.0f;
  replayEnabled      = cfg.enableReplay;
  maxVoicesMixed     = 0;
  maxVolume          = 0;
  mixerDuration      = 0;
  reverbMinDuration  = 0;
  reverbMaxDuration  = 0;

  logger.INFO("Master Volume: %f", masterVolume);

  // The order is critical as some global variables are set here and 
  // used down the road.

  samples   = new SamplesLibrary();
  reverb    = new Reverb();
  poly      = new Poly();
  equalizer = new Equalizer();

  samples->loadLibrary(cfg.samplesLibrariesLocation);

  sound     = new Sound();
  midi      = new Midi();

  signal(SIGINT, sigintHandler);

  logger.INFO("Ready!");
}

//---- ~PIano() ----

/// Rundown method for the PIano application. It removes all object freeing memory allocated then present
/// the statistics cumulated during execution.

PIano::~PIano()
{
  delete midi;
  delete sound;
  delete poly; 
  delete samples;
  delete reverb;
  delete equalizer;
  delete config;

  logger.INFO("Max number of voices mixed at once: %d.", maxVoicesMixed);

  logger.INFO("Max volume: %8.2f.", maxVolume);

  logger.INFO("Max duration of mixer function: %ld nsec (%.0f Hz).", 
	      mixerDuration, 1000000000.0 / mixerDuration);
  logger.INFO("Min duration of reverb function: %ld nsec (%.0f Hz).", 
	      reverbMinDuration, 1000000000.0 / reverbMinDuration);
  logger.INFO("Max duration of reverb function: %ld nsec (%.0f Hz).", 
	      reverbMaxDuration, 1000000000.0 / reverbMaxDuration);
}

//----- outOfMemory() ----

void PIano::outOfMemory()
{
  logger.FATAL("PIano: Unable to allocate memory.");
}

/// Show the Copyright Notice to the end user.

void showCopyright()
{
  using namespace std;

  cout <<
"\n\
 Simplified BSD License\n\
 ----------------------\n\
\n\
 Copyright (c) 2015, Guy Turcotte\n\
 All rights reserved.\n\
 \n\
 Redistribution and use in source and binary forms, with or without\n\
 modification, are permitted provided that the following conditions are met:\n\
 \n\
 1. Redistributions of source code must retain the above copyright notice, this\n\
    list of conditions and the following disclaimer.\n\
 2. Redistributions in binary form must reproduce the above copyright notice,\n\
    this list of conditions and the following disclaimer in the documentation\n\
    and/or other materials provided with the distribution.\n\
 \n\
 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS \"AS IS\" AND\n\
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED\n\
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE\n\
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR\n\
 ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES\n\
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;\n\
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND\n\
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT\n\
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS\n\
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.\n\
 \n\
 The views and conclusions contained in the software and documentation are those\n\
 of the authors and should not be interpreted as representing official policies,\n\
 either expressed or implied, of the FreeBSD Project.\n\
" << endl;
}

//---- MAIN ----

#include <getopt.h>

int main(int argc, char **argv) {

  char opt;

  interactive = false;

  using namespace std;

  while ((opt = getopt(argc, argv, "ihc")) != (char)-1) {
    switch (opt) {
    case 'i':
      interactive = true;
      break;
    case 'c':
      showCopyright();
      exit(0);
      break;
    case 'h':
    case '?':
      cout << endl;
      cout << "Usage: PIano [options...]"    << endl << endl;
      cout << "  -i  Interactive Mode."      << endl;
      cout << "  -c  Show Copyright Notice." << endl;
      cout << "  -h  This help text."        << endl << endl;
      cout << "(c) 2015, Guy Turcotte"       << endl;
      exit(0);
      break;
    }
  }

  pthread_t opener;
  pthread_t feeder;

  piano = new PIano();

  if (piano == NULL) {
    logger.FATAL("Unable to allocate memory for PIano.");
  }
  
  if (pthread_create(&feeder, NULL, samplesFeeder, NULL)) {
    logger.FATAL("Unable to start samplesFeeder thread.");
   }

  if (pthread_create(&opener, NULL, sampleFileOpener, NULL)) {
    logger.FATAL("Unable to start sample_file_opener thread.");
  }
  
  if (interactive) {
    InteractiveMode im;
    im.menu();
    keepRunning = false;
  }

  pthread_join(feeder, NULL);
  pthread_join(opener, NULL);

  delete piano;

  return SUCCESS;
}
