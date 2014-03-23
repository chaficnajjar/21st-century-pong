# Use bash not sh
SHELL := /bin/bash

UNAME := $(shell uname)

ifeq ($(UNAME), Darwin)
	$(shell brew install sdl)
	$(shell brew install sdl_ttf)
endif


GCC = g++
SOURCE = src/main.cpp src/pong.cpp src/ball.cpp src/paddle.cpp
FLAGS = -Wall -g -lSDL2 -lSDL2_ttf -lSDL2_mixer -std=c++0x `sdl2-config --cflags --libs`
BINARY = pong

all:
	$(GCC) $(SOURCE) -o $(BINARY) $(FLAGS)
clean:
	@rm *.o
	@rm $(BINARY)
