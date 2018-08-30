obj-m = coldsim.o

all:
	g++ -w ColdSim.cpp -o coldsim.o -std=c++0x
run:
	./coldsim.o
clean:
	rm coldsim.o