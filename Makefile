UNAME := $(shell uname)

ifeq ($(UNAME), Darwin)
	brew install sdl
	brew install sdl_ttf
endif

GCC = g++
SOURCE = src/main.cpp src/pong.cpp src/ball.cpp src/paddle.cpp
FLAGS = -lSDL2 -lSDL2_ttf -lSDL2_mixer -std=c++0x `sdl2-config --cflags --libs`
BINARY = pong

all:
	$(GCC) $(SOURCE) -o $(BINARY) $(FLAGS)

clean:
	@rm *.o
	@rm $(BINARY)
