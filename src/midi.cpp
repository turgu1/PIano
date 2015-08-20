#include "copyright.h"

#include <iomanip>

#include "PIano.h"

PRIVATE const int MIDI_EVENT_COUNT  = 50;

PRIVATE const int MIDI_STATUS_MASK  = 0x80;

PRIVATE const int MIDI_CHANNEL_MASK = 0x0F;
PRIVATE const int MIDI_COMMAND_MASK = 0xF0;

PRIVATE const int MIDI_NOTE_OFF     = 0x80;
PRIVATE const int MIDI_NOTE_ON      = 0x90;
PRIVATE const int MIDI_POLY_AT      = 0xA0;
PRIVATE const int MIDI_CONTROL      = 0xB0;
PRIVATE const int MIDI_PROGRAM      = 0xC0;
PRIVATE const int MIDI_CHANNEL_AT   = 0xD0;
PRIVATE const int MIDI_PITCHBEND    = 0xE0;

PRIVATE const int MIDI_SYSEX        = 0xF0;
PRIVATE const int MIDI_MTC          = 0xF1;
PRIVATE const int MIDI_SONGPOS      = 0xF2;
PRIVATE const int MIDI_SONGSEL      = 0xF3;
PRIVATE const int MIDI_TUNE         = 0xF6;
PRIVATE const int MIDI_EOX          = 0xF7;
PRIVATE const int MIDI_CLOCK        = 0xF8;
PRIVATE const int MIDI_F9           = 0xF9;
PRIVATE const int MIDI_START        = 0xFA;
PRIVATE const int MIDI_CONTINUE     = 0xFB;
PRIVATE const int MIDI_STOP         = 0xFC;
PRIVATE const int MIDI_FD           = 0xFD;
PRIVATE const int MIDI_ACTIVE       = 0xFE;
PRIVATE const int MIDI_RESET        = 0xFF;

void midiCallBack (double timeStamp, 
		   std::vector<unsigned char> * message, 
		   void * userData)
{
  //DEBUG("midiCallBack() ...\n");

  (void) timeStamp;
  (void) userData;

  int count = message->size();

  if ((count <= 0) || (count > 3)) {
    logger.ERROR("midiCallBack, reveived %d bytes, 1, 2 or 3 expected.", count);
    if (count > 0) logger.ERROR("Midi command: %d (%xh).", 
				message->at(0), message->at(0));
    return;
  }

  int channel = message->at(0) & MIDI_CHANNEL_MASK;

  unsigned char command = message->at(0) & ((unsigned char) MIDI_COMMAND_MASK);
  unsigned char data1 = (count > 1) ? message->at(1) : 0;
  unsigned char data2 = (count > 2) ? message->at(2) : 0;

  if (command == MIDI_SYSEX) command = message->at(0);

  if (((command & 0xF0) == MIDI_SYSEX) ||
      ((1 << channel) & midi->channelMask)) {

    switch (command) {
    case MIDI_NOTE_ON:
      midi->setNoteOn(data1, data2);
      break;
    case MIDI_NOTE_OFF:
      midi->setNoteOff(data1, data2);
      break;
    case MIDI_CONTROL:
      switch (data1) {
      case 0x40:
	if (midi->pedalIsOn() && (data2 < 64)) {
	  midi->setPedalOff();
	}
	else if (!midi->pedalIsOn() && (data2 >= 64)) {
	  midi->setPedalOn();
	}
	break;
      case 0x47:
	reverb->setRoomSize(0.7 + 0.29 * (data2 / 127.0f));
	break;
      case 0x4A:
	masterVolume = data2 / 127.0f;
	break;
      default:
	logger.WARNING("Midi: Ignored Control: %02xh %d.\n",
		       data1, data2);
	break;
      }
      break;
    default:
      logger.WARNING("Midi: Ignored Event: %02xh %d %d.\n",
		     command, data1, data2);
      break;
    }
  }
}


Midi::Midi()
{
  using namespace std;

  int devCount;
  int devNbr = -1;

  pedalOn = false;
  midiPort = NULL;

  try {
    midiPort = new RtMidiIn();
  }
  catch (RtMidiError &error) {
    logger.FATAL("Unable to Initialize Midi: %s.", error.what());
  }

  devCount = midiPort->getPortCount();

  cout << endl << endl;
  cout << "MIDI Input Device list:" << endl;
  cout << "----------------------"  << endl;

  for (int i = 0; i < devCount; i++) {
    cout << "Device " << i << ": " <<  midiPort->getPortName(i) << endl;

    const char * name = midiPort->getPortName(i).c_str();
    if ((devNbr == -1) && 
	(strcasestr(name, cfg.midi.deviceName) != NULL)) {
      devNbr = i;
    }
  }
  cout << "[End of list]" << endl << endl;

  devNbr = cfg.midi.deviceNbr == -1 ? devNbr : cfg.midi.deviceNbr;

  if (devNbr == -1) {
    devNbr = 0;
    if (!interactive) logger.INFO("Default Midi Device (0) selected.");
  }
  else {
    if (!interactive) logger.INFO("MIDI Device Selected: %d.", devNbr);;
  }

  if (interactive) {
    while (true) {
      char userData[6];
      int userNbr;
      cout << "Please enter MIDI device number to use: [Default: " << devNbr << "]> ";
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
    cout << "MIDI Device Selected: " << devNbr << endl << endl;
  }
    
  try {
    midiPort->setCallback(&midiCallBack);
  }
  catch (RtMidiError &error) {
    logger.FATAL("Unable to set Midi CallBack: %s.", error.what());
  }

  try {
    midiPort->openPort(devNbr, "PIano Midi Port");
  }
  catch (RtMidiError &error) {
    logger.FATAL("Unable to open MIDI Device: %s.", error.what());
  }


  if (cfg.midi.channel == -1) {
    logger.INFO("Listening to all MIDI channels.");
  } 
  else {
    char data[60];
    char comma[2] = " ";
    char val[10];

    data[0] = 0;
    for (int i = 0; i < 16; i++) {
      if (cfg.midi.channel & (1 << i)) {
	sprintf(val, "%d", i + 1);
	strcat(data, comma);
	strcat(data, val);
	comma[0] = ',';
      }
    }
    logger.INFO("Listening to MIDI channels%s.", data);
  }

  channelMask = cfg.midi.channel;
}

void Midi::setNoteOn(char note, char velocity)
{
  //DEBUG("Note ON %d (%d)\n", note, velocity);

  if (replayEnabled && (note == 108)) {
    sound->toggleReplay();
  }
  else {
    if (velocity == 0) {
      poly->noteOff(note, pedalOn);
    }
    else {
      samplep sample = samples->getNote(note, velocity);
      if (sample != NULL) {
	poly->addVoice(sample, note, (float) velocity / sample->getVolume());
      }
    }
  }
}

void Midi::setNoteOff(char note, char velocity)
{
  //DEBUG("Note OFF %d (%d)\n", note, velocity);

  (void) velocity;

  poly->noteOff(note, pedalOn);
}

void Midi::setPedalOn()
{
  //DEBUG("Pedal ON\n");

  pedalOn = true;
}

void Midi::setPedalOff()
{
  //DEBUG("Pedal OFF\n");

  if (pedalOn) {
    pedalOn = false;
    poly->voicesSustainOff();
  }
}

Midi::~Midi()
{
  //DEBUG("midi_close() ...\n");

  if (midiPort->isPortOpen()) midiPort->closePort();
  if (midiPort != NULL) delete midiPort;
}
