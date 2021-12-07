all: prog

prog: main.o 
	g++ main.o -o prog


# complex.out: main.o complex.o
# 	g++ complex.o main.o -o complex.out

# main.o: main.cpp complex.h
# 	g++ -c main.cpp -o main.o

# complex.o: complex.cpp complex.h
# 	g++ -c complex.cpp  -o complex.o

clean:
	rm -f prog
	rm -f *.o
