BINARY          := pong
SRCS            := $(wildcard src/*.cpp)
OBJS            := ${SRCS:.cpp=.o}

DEBUG           := -g

SDL_INCLUDE     := `sdl2-config --cflags`
SDL_LIB         := `sdl2-config --libs` -lSDL2_ttf -lSDL2_mixer

CPPFLAGS        := $(SDL_INCLUDE)
CXXFLAGS        := $(DEBUG) -Wall -std=c++11
LDFLAGS         := $(SDL_LIB)

.PHONY: all clean

all: $(BINARY)

$(BINARY): $(OBJS)
	$(LINK.cc) $(OBJS) -o $(BINARY) $(LDFLAGS)

-depend: src/.depend

.generate_depend: $(SRCS)
	@- $(RM) ./src/.depend
	@- $(CXX) $(CPPFLAGS) $(CXXFLAGS) -MM $^ > ./src/.depend;

-include .generate_depend

clean:
	@- $(RM) $(BINARY)
	@- $(RM) $(OBJS)
	@- $(RM) src/.depend

