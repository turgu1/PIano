CC=g++

CFLAGS = -std=c++0x -pthread -c -W -Wall -Wextra -pedantic -Wno-char-subscripts \
         -Wno-unused-function -O4 -march=armv7-a -mfloat-abi=hard -mfpu=neon \
         -D__LINUX_ALSA__

#CFLAGS = -std=c++0x -pthread -c -W -Wall -Wextra -pedantic -Wno-char-subscripts \
#         -Wno-unused-function -O0 -march=armv7-a -mfloat-abi=hard -mfpu=neon \
#         -D__LINUX_ALSA__ -g

#LDFLAGS = -lsndfile -lportaudio -lasound -pthread -lm -lrt -g
LDFLAGS = -lsndfile -lportaudio -lasound -pthread -lm -lrt

MAKEDEPEND = $(CC) -M $(CFLAGS) -o $(df).d $<
DEPDIR = .deps
df = $(DEPDIR)/$(*F)

SRCDIR = src
OBJDIR = obj

vpath %.h   $(SRCDIR)
vpath %.cpp $(SRCDIR)
vpath %.o   $(OBJDIR)

SOURCES = config.cpp main.cpp sound.cpp poly.cpp midi.cpp RtMidi.cpp log.cpp \
          samples_library.cpp sample.cpp reverb.cpp duration.cpp utils.cpp \
          fifo.cpp voice.cpp equalizer.cpp format.cpp interactive_mode.cpp

EXECUTABLE=PIano

all: $(SOURCES) $(EXECUTABLE)

OBJECTS=$(patsubst %,$(OBJDIR)/%,$(SOURCES:.cpp=.o))
    
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $^ $(LDFLAGS) -o $@

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	@$(MAKEDEPEND); \
	  cp $(df).d $(df).P; \
	  sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' \
	      -e '/^$$/ d' -e 's/$$/ :/' < $(df).d >> $(df).P; \
	  rm -f $(df).d
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm $(OBJDIR)/*.o
	rm $(DEPDIR)/*

tags:
	etags src/*.h src/*.cpp

doc:
	doxygen Doxyfile

-include $(SOURCES:%.cpp=$(DEPDIR)/%.P)
