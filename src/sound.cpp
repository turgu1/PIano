#include "copyright.h"

#include <stdio.h>
#include <stdlib.h>
#include <sched.h>

#include <string.h>
#include <iomanip>

#include "PIano.h"

//---- soundCallback() ----

int soundCallback(const void *                     inputBuffer,
		  void *                           outputBuffer,
		  unsigned long                    framesPerBuffer,
		  const PaStreamCallbackTimeInfo * timeInfo,
		  PaStreamCallbackFlags            statusFlags,
		  void *                           userData)
{
  buffp buff = (buffp) outputBuffer;
  
  (void) inputBuffer; /* Prevent "unused variable" warnings. */
  (void) userData;
  (void) timeInfo;
  (void) statusFlags;

  if (replayEnabled && sound->isReplaying()) {
    sound->get(buff);
  }
  else {
    poly->mixer(buff, framesPerBuffer);
    reverb->process(buff, framesPerBuffer);
    equalizer->process(buff, framesPerBuffer);
    if (replayEnabled) sound->push(buff);
  }

  return paContinue;
}

//---- Sound() ----

#define CHKPA(stmt, msg) \
  if ((err = stmt) < 0) { logger.FATAL(msg, Pa_GetErrorText(err)); }

Sound::Sound() 
{
  setNewHandler(outOfMemory);

  using namespace std;

  int devCount;
  int err;

  replay = false;

  PaStreamParameters params;
  PaStreamFlags      flags;

  const PaDeviceInfo *devInfo;
  int devNbr = -1;

  stream = NULL;

  CHKPA(Pa_Initialize(), "Unable to initialize PortAudio: %s");

  if ((devCount = Pa_GetDeviceCount()) < 0) {
    logger.FATAL("Unable to get audio device count: %s.", Pa_GetErrorText(devCount));
  }

  if (!silent) {
    cout << endl << endl;
    cout << "PCM Device list:" << endl;
    cout << "---------------"  << endl;
  }

  for (int i = 0; i < devCount; i++) {
    devInfo = Pa_GetDeviceInfo(i);

    if (!silent) cout << "Device " << i << ": " << devInfo->name << endl;

    if ((devNbr == -1) && (strcasestr(devInfo->name, cfg.pcm.deviceName) != NULL)) {
      devNbr = i;
    }
  }

  if (!silent) cout << "[End of list]" << endl << endl;

  devNbr = cfg.pcm.deviceNbr == -1 ? devNbr : cfg.pcm.deviceNbr;

  if (devNbr == -1) {
    devNbr = 0;
    if (!interactive) logger.INFO("Default PCM Device (0) selected.");
  }
  else {
    if (!interactive) logger.INFO("PCM Device Selected: %d.", devNbr);
  }

  if (interactive) {
    while (true) {
      char userData[6];
      int userNbr;
      cout << "Please enter PCM device number to use: [Default: " << devNbr << "]> ";
      cin >> setw(5) >> userData;
      if (strlen(userData) == 0) break;
      userNbr = atoi(userData);
      if ((userNbr < 0) || (userNbr >= devCount)) {
	cout << "!! Invalid device number[" << userNbr << "]. Please try again !!" << endl;
      }
      else {
	devNbr = userNbr;
	break;
      }
    }
    cout << "PCM Device Selected: " << devNbr << endl << endl;
  }

  params.device       = devNbr;
  params.channelCount = 2;

  params.sampleFormat = paFloat32;
  flags = 0;

  params.suggestedLatency = Pa_GetDeviceInfo(params.device)->defaultLowOutputLatency;
  params.hostApiSpecificStreamInfo = NULL;

  CHKPA(Pa_OpenStream(&stream, 
		      NULL, &params, samplingRate, 
		      BUFFER_FRAME_COUNT, flags, &soundCallback, NULL), 
	"Unable to open PortAudio Stream: %s");

  CHKPA(Pa_StartStream(stream), 
	"Unable to start PortAudio Stream: %s");

  // Initialize replay buffer
  rbuff = new sample_t[REPLAY_BUFFER_SAMPLE_COUNT];

  memset(rbuff, 0, REPLAY_BUFFER_SIZE);

  rhead = rtail = rbuff;
  rend  = &rbuff[REPLAY_BUFFER_SAMPLE_COUNT];
}

//---- ~Sound() ----
 
Sound::~Sound()
{
  if (stream) {
    Pa_AbortStream(stream);
    Pa_CloseStream(stream);
  }

  delete [] rbuff;

  Pa_Terminate();
}

//----- outOfMemory() ----

void Sound::outOfMemory()
{
  logger.FATAL("Sound: Unable to allocate memory.");
}

