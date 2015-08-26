#include "copyright.h"

#ifndef SAMPLES_H
#define SAMPLES_H

#include "sample.h"

// Music Velocity Midi equivalencies
//
// ppp: 16    Piano Pianissimo
// pp: 32     Pianissimo
// p: 48      Piano
// mp: 64     Mezzo-piano
// mf: 80     Mezzo-forte
// f: 96      forte
// ff: 112    fortissimo
// fff: 127   forte fortissimo
//

/// Class SamplesLibrary is responsible of managing all samples that are part
/// of a library, insuring initial load of the first part of each sample data and
/// supplying the appropriate sample when a note to be played is requested by the
/// user. It maintains a vector of all potential notes (128 MIDI values) with, for
/// each note, the linked list of samples in the sort order of volume (or gain, or 
/// attack level).

class SamplesLibrary : public NewHandlerSupport<SamplesLibrary> {

 private:
  samplep notes[128]; ///< Samples are linked to the appropriate note entry in this vector

  void   addToNotes(samplep sample); ///< Add a sample to a note
  void   clearDB();                  ///< Clear the library
  char * ext(char * fname);          ///< Extract filename extension

  static void outOfMemory();         ///< New operation handler when out of memory occurs

  struct libStruct {
    char * dirName;
    struct libStruct * next;
  } * libs;
  struct libStruct * currentLib;

  /// Load the library at location. This method uses the SndFile package to read
  /// sample files into memory.
  int loadLibrary(char * location);

 public:
   SamplesLibrary();
  ~SamplesLibrary();

  void loadFirstLibrary();
  void loadNextLibrary();

  /// Returns the sample corresponding to the note and volume requested. Is a sample
  /// is not available for the note, the sample for a lower closest note will be returned,
  /// allowinf for further processing (scaling).
  samplep getNote(char note, char volume);

  /// Display the library structure to the user
  void   showNotes();

};

#endif
