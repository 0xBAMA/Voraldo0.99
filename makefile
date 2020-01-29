all: voraldo.o main.o
	g++ -o main main.o voraldo.o
	rm main.o voraldo.o

main.o: main.cc voraldo.o
	g++ -c -o main.o main.cc

voraldo.o: voraldo.h voraldo.cc
	g++ -c -o voraldo.o voraldo.cc
