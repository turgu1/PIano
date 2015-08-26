#include "copyright.h"

#include <string.h>
#include <stdlib.h>
#include <sndfile.h>
#include <ctype.h>
#include <dirent.h>

#include "PIano.h"
#include "format.h"

//---- clearDB() ----

void SamplesLibrary::clearDB()
{
  int i;

  //logger.INFO("--> clearDB()");

  for (i = 0; i < 128; i++) {
    samplep s = notes[i];
    notes[i] = NULL;
    while (s != NULL) {
      samplep s1 = s->getNext();
      delete s;
      s = s1;
    }
  }
}

//---- SamplesLibrary() ----

SamplesLibrary::SamplesLibrary()
{
  setNewHandler(outOfMemory);

  for (int i = 0; i < 128; i++) notes[i] = NULL;

  libs = NULL;

  DIR * dir = opendir(cfg.samplesLibrariesLocation);
  struct dirent * file;

  if (dir != NULL) {

    logger.INFO("Reading samples library folders at location %s.", cfg.samplesLibrariesLocation);

    while ((file = readdir(dir)) != NULL) {

      if ((file->d_type == DT_DIR) && 
	  (strcmp(".", file->d_name) != 0) && 
	  (strcmp("..", file->d_name) != 0)) {

	struct libStruct * l = (struct libStruct *) malloc(sizeof(struct libStruct));

	l->dirName = (char *) malloc(strlen(cfg.samplesLibrariesLocation) + strlen(file->d_name) + 2);
	strcpy(l->dirName, cfg.samplesLibrariesLocation);
	strcat(l->dirName, "/");
	strcat(l->dirName, file->d_name);
	
	l->next = NULL;
	if (libs == NULL) {
	  libs = l;
	}
	else {
	  struct libStruct * n = libs;
	  struct libStruct * n2 = NULL;
	  while (n && (strcmp(n->dirName, l->dirName) < 0)) {
	    n2 = n;
	    n = n->next;
	  }
	  if (n2 != NULL) n2->next = l;
	  l->next = n;
	}
      }
    }

    closedir(dir);
  }

  currentLib = libs;
}

//---- ~SamplesLibrary() ----

SamplesLibrary::~SamplesLibrary()
{
  clearDB();

  struct libStruct * l = libs;
  while (l) {
    free(l->dirName);
    struct libStruct * n = l->next;
    free(l);
    l = n;
  }
}

//---- loadFirstLibrary() ----

void SamplesLibrary::loadFirstLibrary()
{
  currentLib = libs;
  clearDB();
  loadLibrary(currentLib->dirName);
}

//---- loadFirstLibrary() ----

void SamplesLibrary::loadNextLibrary()
{
  //logger.INFO("--> loadNextLibrary()");

  currentLib = currentLib->next;
  if (currentLib == NULL) currentLib = libs;
  clearDB();
  loadLibrary(currentLib->dirName);
}

//----- outOfMemory() ----

void SamplesLibrary::outOfMemory()
{
  logger.FATAL("SamplesLibrary: Unable to allocate memory.");
}

//---- addToNotes() ----

void SamplesLibrary::addToNotes(samplep sample)
{
  samplep n = notes[sample->getNote()];
  samplep p = NULL;

  while ((n != NULL) && (n->getVolume() < sample->getVolume())) { 
    p = n; 
    n = n->getNext(); 
  }

  sample->setNext(n);

  if (p == NULL) {
    notes[sample->getNote()] = sample;
  }
  else {
    p->setNext(sample);
  }
}

//---- showNotes() ----
void SamplesLibrary::showNotes()
{
  using namespace std;

  cout << "[Samples Library Dump Start]" << endl;

  for (int i = 0; i < 128; i++) {
    cout << "Note " << i << "(" << valueToNote(i) << ")" << endl;
    samplep p = notes[i];
    while (p != NULL) {
      cout << "  " << p->getVolume() << " - " <<  p->getFilename() << endl;
      p = p->getNext();
    }
  }
  cout << "[Samples Library Dump End]" << endl << endl;
}

//---- ext() ----
//
// Extract file extension from filename. The extension is expected to have less
// than 10 characters. If no extension present, return NULL, else a pointer to
// a string containing the extension. 
//
// The returned pointer must be used before next call to this function. 

char * SamplesLibrary::ext(char * fname)
{
  static char ex[10];

  char *s = ex;
  char *p = strrchr(fname, '.');
  int   i = 0;

  if (p == NULL) return NULL;
  while ((i++ < 9) && (*p)) *s++ = tolower(*p++);
  *s = '\0';

  return ex;
}

//---- loadLibrary() ----

int SamplesLibrary::loadLibrary(char * location)
{
  int count = 0;

  //logger.DEBUG("--> loadLibrary()");

  if (load_format_file(location)) return -1;

  DIR * dir = opendir(location);
  struct dirent * file;

  logger.INFO("Reading samples from location %s.", location);

  if (dir != NULL) {

    while ((file = readdir(dir)) != NULL) {

      if (file->d_type == DT_REG) {
	char * ex = ext(file->d_name);

	if ((strcmp(ex, ".ogg") == 0) || 
	    (strcmp(ex, ".wav") == 0) ||
	    (strcmp(ex, ".flac") == 0)) {

          char note, volume;

          if (parse_filename(file->d_name, &note, &volume) == SUCCESS) {

            char filename[1024];
            strcpy(filename, location);
            strcat(filename, "/");
            strcat(filename, file->d_name);

	    samplep sample = new Sample(filename, note, volume);

	    addToNotes(sample);
	    count += 1;
          }
          else {
            logger.WARNING("Unable to add file %s to library", file->d_name);
          }
	}
        else if (strcmp(ex, ".txt") != 0) {
          logger.WARNING("Unknown file: %s"
	               "Only files with extension .txt, .wav, .flac or .ogg "
                       "are processed in a library folder.\n", file->d_name);
        }
      }
    }

    closedir(dir);
  }
  else {
    logger.ERROR("Unable to open samples location %s", location);
    return -1;
  }

  logger.INFO("%d Samples Loaded.", count);

  return SUCCESS;
}

//---- getNote() ----

samplep SamplesLibrary::getNote(char note, char volume)
{
  samplep sample;
  sample = notes[note];
  while ((sample == NULL) && (note > 0)) sample = notes[--note];

  if (sample != NULL) {
    while ((sample->getNext() != NULL) && (sample->getNext()->getVolume() <= volume)) {
      sample = sample->getNext();
    }
  }

  return sample;
}
