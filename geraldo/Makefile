OPTIONS = -c -O3
COMP = g++

MDHFVRP: data.o main.o
	$(COMP) data.o main.o -o MDHFVRP

data.o: data.cpp data.h
	$(COMP) $(OPTIONS) data.cpp data.h

main.o:
	$(COMP) $(OPTIONS) main.cpp

clean:
	rm *.o MDHFVRP
