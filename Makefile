CC = g++
SRCS = src/main.cpp src/pong.cpp src/ball.cpp src/paddle.cpp src/utilities.cpp
OBJS = main.o pong.o ball.o paddle.o utilities.o
DEBUG = -g
SDL_LIB = -lSDL2 -lSDL2_ttf -lSDL2_mixer `sdl2-config --libs`
SDL_INCLUDE = `sdl2-config --cflags`
CFLAGS = -Wall -c -std=c++0x $(DEBUG) $(SDL_INCLUDE)
LFLAGS = -Wall $(DEBUG) $(SDL_LIB)
BINARY = pong

$(BINARY): $(OBJS)
	$(CC) $(OBJS) -o $(BINARY) $(LFLAGS)

main.o: src/main.cpp
	$(CC) -c src/main.cpp -o main.o $(CFLAGS)

pong.o: src/pong.cpp
	$(CC) -c src/pong.cpp -o pong.o $(CFLAGS)

ball.o: src/ball.cpp
	$(CC) -c src/ball.cpp -o ball.o $(CFLAGS)

paddle.o: src/paddle.cpp
	$(CC) -c src/paddle.cpp -o paddle.o $(CFLAGS)

utilities.o: src/utilities.cpp
	$(CC) -c src/utilities.cpp -o utilities.o $(CFLAGS)

depend: .depend

.depend: $(SRCS)
	rm -f ./.depend
	$(CC) -MM $^ $(CFLAGS) > ./.depend;

-include .depend

clean:
	rm -rf *.o .depend $(BINARY)
