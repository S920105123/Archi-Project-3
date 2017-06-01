all: simulator.o datapath.o loader.o const.o memsys.o
	g++ -std=c++11 simulator.o datapath.o loader.o const.o memsys.o -O3 -o CMP

memsys.o:
	g++ -std=c++11 memsys.hpp memsys.cpp -O3 -c

simulator.o:
	g++ -std=c++11 simulator.cpp -O3 -c

datapath.o:
	g++ -std=c++11 datapath.hpp datapath.cpp -O3 -c

loader.o:
	g++ -std=c++11 loader.hpp loader.cpp -O3 -c

const.o:
	g++ -std=c++11 const.hpp const.cpp -O3 -c

clean:
	rm *.o *.gch *.stack_dump CMP
