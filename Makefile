BINARY          := pong
SRCS            := $(wildcard src/*.cc)
OBJS            := $(SRCS:.cc=.o)

DEBUG           := -g

SDL_INCLUDE     := `sdl2-config --cflags`
SDL_LIB         := `sdl2-config --libs` -lSDL2_ttf -lSDL2_mixer

CPPFLAGS        += $(SDL_INCLUDE) -I.
CXXFLAGS        += $(DEBUG) -Wall -std=c++11
LDFLAGS         += $(SDL_LIB)

.PHONY: all clean

all: $(BINARY)

$(BINARY): $(OBJS)
	$(LINK.cc) $(OBJS) -o $(BINARY) $(LDFLAGS)

.depend: $(SRCS)
	@- $(RM) .depend
	@- $(CXX) $(CPPFLAGS) $(CXXFLAGS) -MM $^ | sed -r 's|^([^ ])|src/\1|' > .depend;

-include .depend

clean:
	@- $(RM) $(BINARY)
	@- $(RM) $(OBJS)
	@- $(RM) .depend

