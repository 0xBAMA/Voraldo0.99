LODEPNG_FLAGS = -ansi -O3 -std=c++11
VORALDO_FLAGS = -std=c++11 -lGLEW -lGL -lGLU $(shell pkg-config sdl2 --cflags --libs)

all: lodepng.o voraldo.o main.o
	g++ -o main main.o voraldo.o
	rm *.o

main.o: main.cc voraldo.o
	g++ -c -o main.o main.cc

voraldo.o: voraldo.h voraldo.cc
	g++ -c -o voraldo.o voraldo.cc ${VORALDO_FLAGS}

lodepng.o: lodepng.h lodepng.cc
	g++ -c -o lodepng.o lodepng.cc ${LODEPNG_FLAGS}
