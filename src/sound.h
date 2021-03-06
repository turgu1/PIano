#include "copyright.h"

#ifndef SOUND_H
#define SOUND_H

#include <portaudio.h>

/// The Sound class is responsible of supplying the proper linkage
/// with the PCM sound device. It uses the PortAudio library to
/// get access to the ALSA API of the operating system. Through the configuration file,
/// the user identify the PCM device to connect with. Once connected, the class setup
/// a callback function that will be asynchronously requesting new PCM data from the
/// application.
///
/// The class maintains a ~5 seconds replay buffer that could be requested to be played
/// by the user. It contains the last 5 seconds of PCM data generated and transmitted to
/// the PCM device and is used to debug samples libraries or code change in this program.
///
/// Getting the connection properly setup with the requested device is a somewhat difficult
/// exercise for the user. The class when started produce the list of compatible devices
/// and show the list to the end user. It is then possible for the user, in the interactive
/// mode, to select the targeted device and produce the data required in the configuration 
/// file to properly setup the Sound class.

class Sound : public NewHandlerSupport<Sound> {

 private:
  PaStream *stream;  ///< The connection to the PortAudio stream
  
  #define REPLAY_BUFFER_SIZE (860 * FRAME_BUFFER_SIZE)  ///< Replay buffer size in bytes
  #define REPLAY_FRAME_COUNT (860 * BUFFER_FRAME_COUNT) ///< Replay buffer size in frames
  #define REPLAY_BUFFER_SAMPLE_COUNT (REPLAY_FRAME_COUNT * 2) ///< Number of PCM samples in the replay buffer

  bool hold;   ///< If true, stop asking form samples as a new library is being loaded
  bool replay; ///< Set to true is replaying is required by the user

  buffp rbuff; ///< FIFO Buffer on the last ~5 seconds PCM data
  buffp rhead; ///< Head Pointer on the replay buffer
  buffp rtail; ///< Tail Pointer on the replay buffer
  buffp rend;  ///< Address that follows the end of the rbuff space
  buffp rpos;  ///< Current replay position

  static void outOfMemory(); ///< New operation handler when out of memory occurs

  /// This is the callback function that will retrieve and process PCM data to be
  /// sent back to the PCM ALSA device. This function call the Poly::mixer method to get
  /// sampling data, then call the reverb and the equalizer filters in sequence to process
  /// the data before sending it to the PCM device. The data is also pushed in the FIFO
  /// replay buffer.
  ///
  /// This routine will be called by the PortAudio engine when audio is needed.
  /// It may be called at interrupt level on some machines so don't do anything
  /// that could mess up the system like calling memory allocation functions that would
  /// disrupt the data structure involved.

  friend int soundCallback(const void *                     inputBuffer,
			   void *                           outputBuffer,
			   unsigned long                    framesPerBuffer,
			   const PaStreamCallbackTimeInfo * timeInfo,
			   PaStreamCallbackFlags            statusFlags,
			   void *                           userData);
 public:
  /// This method selects which PCM device to connect to, establish the connection and
  /// setup the callback function.
   Sound();
  ~Sound();

  /// Show available devices to user
  void showDevices(int devCount);

  /// Interactive device selection
  void deviceSelect();

  bool holding() { return hold; }

  /// Put sound on hold waiting for a new sample library to be loaded from disk
  void wait() { hold = true; }

  /// Restart sound, the new library has been loaded
  void conti() { hold = false; }

  /// This is called when starting a replay process. The rpos pointer is adjusted to the
  /// tail of the FIFO buffer.
  inline void startReplay() { 
    rpos = rtail; 
  }

  /// Turn on and off the replay mechanism
  inline void toggleReplay() { 
    replay = !replay;
    if (replay) startReplay();
  }

  /// Returns true if the class is currently in a replay mode, false otherwise.
  inline bool isReplaying() { return replay; }

  /// Push in the replay buffer the just prepared PCM data. As the replay buffer is managed
  /// as a FIFO, it keep only the last 5 seconds of musical data.
  inline void push(buffp b) {
    memcpy(rhead, b, FRAME_BUFFER_SIZE);
    rhead += REPLAY_BUFFER_SAMPLE_COUNT;
    if (rhead >= rend) rhead = rbuff;
    rtail = rhead;
  }

  /// Retrieval of a chunk of PCM data from the replay buffer to send to the PCM device.
  inline void get(buffp b) {
    memcpy(b, rpos, FRAME_BUFFER_SIZE);
    rpos += REPLAY_BUFFER_SAMPLE_COUNT;
    if (rpos >= rend) rpos = rbuff;
  }
};

#endif
