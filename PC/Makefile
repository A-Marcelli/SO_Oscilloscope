
.phony:	clean all

all: main_pc

main_pc: main_pc.o serial_linux.o
	gcc main_pc.o serial_linux.o -o main_pc

main_pc.o: main_pc.c serial_linux.h
	gcc -c main_pc.c 

serial_linux.o: serial_linux.c serial_linux.h
	gcc -c serial_linux.c

clean:
	rm main_pc *.o