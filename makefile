LODEPNG_FLAGS = -ansi -O3 -std=c++11
VORALDO_FLAGS = $(shell pkg-config sdl2 --cflags --libs) -O3 -std=c++11 -lGLEW -lGL -lGLU

all: msg main

msg:
		@echo 'Compiling starts on '
		@date

main: voraldo.h voraldo.cc lodepng.o
	g++ -o main main.cc voraldo.cc lodepng.o ${VORALDO_FLAGS} -O3 -lSDL2 -lSDL2_ttf

lodepng.o: lodepng.h lodepng.cc
	g++ ${LODEPNG_FLAGS} -c -o lodepng.o lodepng.cc

run: msg
	time ./main
