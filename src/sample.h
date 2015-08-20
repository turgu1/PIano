#include "copyright.h"

#ifndef SAMPLE_H
#define SAMPLE_H

typedef class Sample * samplep;

/// Class Sample represents the static portion of a single note sampled at a specific 
/// gain (volume) level. (Class Voice represents the dynamic portion of it while it is
/// played by the user.) When a sample is created, a first portion of the PCM data is
/// gathered in memory and will be served while PIano setup itself to retrieve the rest of
/// it when required through multithreading.

class Sample : public NewHandlerSupport<Sample> {

 private:
  char    * filename;   ///< The name of the file that contains sampling data
  char      note;       ///< Midi note number associated with this sampling data
  int       volume;     ///< The gain (or volume) level of that sampling data (0..127)
  buffp     buff;       ///< Buffer that receives the first portion of the sample PCM data
  int       frameCount; ///< How many frames are gathered in the buffer
  samplep   next;       ///< Used to link samples associated to a single note (See SamplesLibrary class)

  static void outOfMemory();  ///< New operation handler when out of memory occurs

 public:
  /// Initialize a sample from the supplied parameters
   Sample(char * fname, char note, int volume);
  ~Sample();
  
  /// Debugging method to show part of the buffer
  void showFrames(int idx, int frameCount);
  /// Debugging method to show the current state of a sample
  void showState();

  /// Set pointer to the next sample in a list
  inline void    setNext(samplep s) { next = s; }

  /// Get next sample in the list
  inline samplep getNext()       { return next;       }

  inline char    getNote()       { return note;       }
  inline int     getVolume()     { return volume;     }
  inline buffp   getBuff()       { return buff;       }
  inline char *  getFilename()   { return filename;   }
  inline int     getFrameCount() { return frameCount; }
};

#endif
