

all: voraldo.o main.o

main.o: main.cc voraldo.o

voraldo.o: voraldo.h voraldo.cc
