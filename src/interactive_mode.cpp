#include "copyright.h"

#include <iomanip>

#include "PIano.h"
#include "interactive_mode.h"


InteractiveMode::InteractiveMode()
{
}

InteractiveMode::~InteractiveMode()
{
}

char InteractiveMode::showMenuGetSelection()
{
  using namespace std;
  char answer[6];

  cout << endl;
  cout << "Menu:" << endl << "----" << endl;

  cout << "a - Monitor active voice count" << endl;
  cout << "m - Monitor midi messages"      << endl;
  cout << "e - Equalizer adjusments"       << endl;
  cout << "r - Reverb adjusments"          << endl;
  cout << "l - dump sample Library"        << endl;
  cout << "v - dump Voices state"          << endl;
  cout << "c - show Config read from file" << endl;
  cout << "x - eXit"                       << endl << endl;

  cout << "Your choice: ";
  cin >> setw(5) >> answer;

  return answer[0];
}

void InteractiveMode::menu()
{

  while (true) {
    char ch = showMenuGetSelection();

    switch (ch) {
    case 'a':
      poly->monitorCount();
      break;
    case 'm':
      midi->monitorMessages();
      break;
    case 'e':
      equalizer->interactiveAdjust();
      break;
    case 'r':
      reverb->interactiveAdjust();
      break;
    case 'l':
      samples->showNotes();
      break;
    case 'v':
      poly->showState();
      break;
    case 'c':
      config->showState();
      break;
    case 'x':
      return;
    default:
      std::cout << "Bad entry!" << std::endl;
      break;
    }
  }
}
