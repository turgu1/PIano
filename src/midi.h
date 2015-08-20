#include "copyright.h"

#ifndef MIDI_H
#define MIDI_H

#include "RtMidi.h"

class Midi {

 private:
  /// This is the callback method called by RtMidi to signify the reception of a new MIDI command
  /// by the application. This callback is responsible of parsing the command and dispatch to modify
  /// PIano voices state accordingly.
  friend void midiCallBack (double timeStamp, 
			    std::vector<unsigned char> *message, 
			    void *userData);
 public:
   Midi();
  ~Midi();
  
 private:
  RtMidiIn * midiPort;  ///< RTMidiIn instance
  bool pedalOn;         ///< True if the sustain pedal is depressed by the user 
  int  channelMask;     ///< Mask of channels being listened by Midi

  void setNoteOn(char note, char velocity);  ///< Process a noteOn MIDI command
  void setNoteOff(char note, char velocity); ///< Process a noteOff MIDI command
  void setPedalOn();
  void setPedalOff();

  bool pedalIsOn() { return pedalOn; };
};

#endif
