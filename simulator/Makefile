all: simulator.o datapath.o loader.o const.o error.o
	g++ -std=c++11 simulator.o datapath.o loader.o const.o error.o -O2 -o single_cycle

simulator.o:
	g++ -std=c++11 simulator.cpp -O3 -c

datapath.o:
	g++ -std=c++11 datapath.hpp datapath.cpp -O3 -c

loader.o:
	g++ -std=c++11 loader.hpp loader.cpp -O3 -c

error.o:
	g++ -std=c++11 error.hpp error.cpp -O3 -c

const.o:
	g++ -std=c++11 const.hpp const.cpp -O3 -c

clean:
	rm *.o *.gch *.stack_dump single_cycle
