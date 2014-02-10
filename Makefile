GCC = g++
SOURCE = pong.cpp
FLAGS = -lSDL2 -lSDL2_ttf -std=c++0x `sdl2-config --cflags --libs`
BINARY = pong

all:
	$(GCC) $(SOURCE) -o $(BINARY) $(FLAGS)

clean:
	@rm *.o
	@rm $(BINARY)
