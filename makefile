LODEPNG_FLAGS = lodepng.cc -ansi -O3 -std=c++11

all: voraldo.o main.o lodepng.o
	g++ -o main main.o voraldo.o
	rm main.o voraldo.o

main.o: main.cc voraldo.o
	g++ -c -o main.o main.cc

voraldo.o: voraldo.h voraldo.cc
	g++ -c -o voraldo.o voraldo.cc

lodepng.o: lodepng.h lodepng.cc
	g++ -c -o lodepng.o ${LODEPNG_FLAGS}
