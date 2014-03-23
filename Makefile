# Use bash not sh
SHELL := /bin/bash

GCC = g++
SOURCE = src/main.cpp src/pong.cpp src/ball.cpp src/paddle.cpp
FLAGS = -Wall -g -lSDL2 -lSDL2_ttf -lSDL2_mixer -std=c++0x `sdl2-config --cflags --libs`
BINARY = pong

all:
	$(GCC) $(SOURCE) -o $(BINARY) $(FLAGS)
	if [ ! "$(uname -s)" = "Darwin" ]; then \
		brew install sdl \
		brew install sdl_ttf \
		brew install sdl_mixer; \
	fi

clean:
	@rm *.o
	@rm $(BINARY)
