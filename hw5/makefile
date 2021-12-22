all: ipscanner

ipscanner: main.o 
	gcc main.o -o ipscanner


util.o: util.c util.h
	gcc -c util.c -o util.o 

pcap.o: pcap.c pcap.h
	gcc -c pcap.c -o pcap.o -lpcap 

fill_packet.o: fill_packet.c fill_packet.h
	gcc -c fill_packet.c -o fill_packet.o 

main.o: main.c
	gcc -c main.c -o main.o  

clean:
	rm -f *.out
	rm -f *.o