BINARY          := pong
SRCS            := $(wildcard src/*.cpp)
OBJS            := ${SRCS:.cpp=.o}

DEBUG           := -g

SDL_LIB         := `sdl2-config --libs` -lSDL2_ttf -lSDL2_mixer
SDL_INCLUDE     := `sdl2-config --cflags`

CPPFLAGS        := $(SDL_INCLUDE)
CXXFLAGS        := -Wall -std=c++11
LDFLAGS         = $(DEBUG) $(SDL_LIB)

all: $(BINARY)

$(BINARY): $(OBJS)
	$(LINK.cc) $(OBJS) -o $(BINARY) $(LDFLAGS)

depend: .depend

.depend: $(SRCS)
	$(RM) ./.depend
	$(CC) $(CPPFLAGS) $(CXXFLAGS) $(DEBUG) -MM $^ > ./.depend;

include .depend

clean:
	$(RM) $(BINARY) $(OBJS) .depend

